(ns jank-build.sandbox.seatbelt
  (:require [clojure.string :as string]
            [babashka.fs :as fs]))

;; Note that sandbox-exec is technically deprecated, but its replacement, App Sandbox, requires
;; a signed application and doesn't provide a per-process policy for an arbitrary CLI build.
;; Until macOS provides a viable replacement, Seatbelt is the only official option.

(def standard-binds
  "macOS paths needed by common build tools and the system runtime.

   Seatbelt does not mount these paths. They are represented as read-only
   allowlist entries in the generated profile. Missing paths are ignored."
  [[:ro-bind "/System" "/System"]
   [:ro-bind "/usr" "/usr"]
   [:ro-bind "/bin" "/bin"]
   [:ro-bind "/sbin" "/sbin"]
   [:ro-bind "/private/etc" "/private/etc"]

   ;; macOS does some weird shit with /private and symlinks in order to select
   ;; things like the current shell. We need to support that.
   [:ro-bind "/private/var/select/sh" "/private/var/select/sh"]
   [:ro-bind "/var/select/sh" "/var/select/sh"]
   [:ro-bind "/private/var/select/developer_dir" "/private/var/select/developer_dir"]
   [:ro-bind "/var/select/developer_dir" "/var/select/developer_dir"]

   [:ro-bind "/Library/Developer/CommandLineTools" "/Library/Developer/CommandLineTools"]
   [:ro-bind "/Applications/Xcode.app/Contents/Developer" "/Applications/Xcode.app/Contents/Developer"]
   [:ro-bind "/Library/Apple/usr" "/Library/Apple/usr"]

   [:ro-bind "/opt/homebrew" "/opt/homebrew"]
   [:ro-bind "/usr/local" "/usr/local"]
   [:ro-bind "/nix" "/nix"]])

(def standard-executable-paths
  "Directories from which build tools may be executed."
  ["/usr/bin"
   "/usr/sbin"
   "/bin"
   "/sbin"
   "/Library/Developer/CommandLineTools"
   "/Applications/Xcode.app/Contents/Developer"
   "/opt/homebrew"
   "/usr/local"
   "/nix"])

(def sandbox-exec-path "/usr/bin/sandbox-exec")

(def default-options {:ro-binds [] :writable-paths [] :scratch-paths [] :network? false})

(defn which-sandbox-exec
  "Find Apple's system `sandbox-exec` executable, or nil if it is unavailable."
  []
  (fs/which sandbox-exec-path))

(defn escape-scheme-string [value]
  (clojure.string/escape (str value) {\\ "\\\\"
                                          \" "\\\""
                                          \newline "\\n"
                                          \return "\\r"
                                          \tab "\\t"}))

(defn path-variants
  "Returns the distinct path strings that a Seatbelt filter for `path` must
   list to reliably match. This covers the cases where paths like /var/folders
   and /private/var/folders both need to be supported, but we don't want to
   have to enumerate all of these ourselves."
  [path]
  (distinct [(str (fs/absolutize path)) (str (fs/canonicalize path))]))

(defn ancestor-dirs
  "Returns the ancestor directories of `path`, from its immediate parent up
   to (and including) the filesystem root."
  [path]
  (loop [dir (fs/parent path)
         acc []]
    (if (nil? dir)
      acc
      (recur (fs/parent dir) (conj acc (str dir))))))

(defn path-filter [path]
  (let [variants (path-variants path)
        filter-kind (if (fs/directory? path) "subpath" "literal")]
    (str "(" filter-kind " "
         (string/join " " (map #(str "\"" (escape-scheme-string %) "\"") variants))
         ")")))

(defn allow-rule [operations path]
  (str "(allow " (string/join " " operations) " " (path-filter path) ")"))

(defn allow-literal-rule
  "Like `allow-rule`, but always emits a `literal` filter, even for
   directories. Used for ancestor-directory metadata grants, which must not
   widen into a `subpath` grant covering the whole subtree."
  [operations path]
  (str "(allow " (string/join " " operations) " (literal \""
       (escape-scheme-string path) "\"))"))

(defn ensure-option-args [kind args expected-count]
  (when-not (= expected-count (count args))
    (throw
      (IllegalArgumentException.
        (format "Sandbox option %s expects %d argument(s), got %d."
                kind
                expected-count
                (count args)))))
  args)

(defn validate-bind! [kind src dst]
  (when-not (= (str src) (str dst))
    (throw
      (IllegalArgumentException.
        (format "Seatbelt cannot remap %s from %s to %s."
                kind
                src
                dst)))))

(defn parse-options [cmds]
  (reduce
    (fn [parsed [kind & args]]
      (case kind
        :ro-bind
        (let [[src dst] (ensure-option-args kind args 2)]
          (validate-bind! kind src dst)
          (update parsed :ro-binds conj [src dst]))

        :bind
        (let [[src dst] (ensure-option-args kind args 2)]
          (validate-bind! kind src dst)
          (update parsed :writable-paths conj src))

        :tmpfs
        (let [[dir] (ensure-option-args kind args 1)]
          (when (and (fs/exists? dir)
                     (not (fs/directory? dir)))
            (throw
              (IllegalArgumentException.
                (format "Seatbelt requires a directory for :tmpfs: %s."
                        dir))))
          (-> parsed
              (update :writable-paths conj dir)
              (update :scratch-paths conj dir)))

        :chdir
        (do
          ;; Seatbelt cannot change the working directory. The caller passes
          ;; this value to ProcessBuilder through :dir instead.
          (ensure-option-args kind args 1)
          parsed)

        :net
        (let [[enabled?] (ensure-option-args kind args 1)]
          (assoc parsed :network? (boolean enabled?)))

        (throw
          (IllegalArgumentException.
            (format "Unknown Seatbelt sandbox option: %s." kind)))))
    default-options
    cmds))

(defn profile
  "Build a default-deny Seatbelt profile from sandbox options.

   Valid options are:
   - [:ro-bind src dst]
   - [:bind src dst]
   - [:tmpfs dir]
   - [:chdir dir] (handled by the process working directory)
   - [:net enabled?]"
  [cmds]
  (let [{:keys [ro-binds writable-paths scratch-paths network?]} (parse-options cmds)
        standard-paths (->> standard-binds
                            (map second)
                            (filter fs/exists?)
                            (map str)
                            distinct)
        executable-standard-paths (->> standard-executable-paths
                                       (filter fs/exists?)
                                       (map str)
                                       distinct)
        input-paths (->> ro-binds
                         (map first)
                         (filter fs/exists?)
                         (map str)
                         distinct)
        read-paths (distinct (concat standard-paths input-paths))
        executable-paths (distinct (concat executable-standard-paths input-paths))
        writable-paths (->> writable-paths
                            (filter fs/exists?)
                            (map str)
                            distinct)
        scratch-paths (->> scratch-paths
                           (filter fs/exists?)
                           (map str)
                           distinct)
        ;; Path filters only grant access to their endpoint. The kernel still needs to trudge
        ;; through every ancestor in order to get there. Also, some paths have certain variants
        ;; we want to support. All of this gets expanded out and then deduped for our final
        ;; set of directories which can be accessed only for metadata.
        metadata-dirs (->> (concat read-paths executable-paths writable-paths scratch-paths)
                           (mapcat path-variants)
                           (mapcat ancestor-dirs)
                           distinct)]
    (string/join
      "\n"
      (concat
        ["(version 1)"
         "(deny default)"
         ;; Build tools use these for normal process setup and coordination.
         "(allow process-fork)"
         "(allow sysctl-read)"
         "(allow ipc-posix*)"
         "(allow ipc-sysv*)"
         "(allow signal (target same-sandbox))"
         ;; getpwuid and related libc calls use this specific service.
         "(allow mach-lookup (global-name \"com.apple.system.opendirectoryd.libinfo\"))"
         ;; file-search only reveals whether a name exists in a directory (not
         ;; its contents or metadata), so Apple's own bundled profiles grant it
         ;; unconditionally (e.g. mdworker.sb). It is distinct from
         ;; file-read-metadata and file-write*, and without it directory
         ;; lookups fail on both read (traversing to an allowed path) and
         ;; write (e.g. mkdir checking whether a new entry already exists)
         ;; operations.
         "(allow file-search)"]
        (map #(allow-literal-rule ["file-read-metadata"] %) metadata-dirs)
        [;; dyld reads the contents of the root directory itself while
         ;; locating the shared cache during process startup.
         ;; Without this, dynamically linked binaries abort before main runs.
         "(allow file-read-data (literal \"/\"))"
         ;; Some toolchains probe /dev before opening standard devices.
         "(allow file-read-metadata (literal \"/dev\"))"
         ;; These file descriptors are needed for shell substitution commonly used in
         ;; build scripts.
         "(allow file* (regex #\"^/dev/fd/[0-9]+$\"))"
         "(allow file* (literal \"/dev/null\") (literal \"/dev/random\")"
         "  (literal \"/dev/stderr\") (literal \"/dev/stdin\")"
         "  (literal \"/dev/stdout\") (literal \"/dev/urandom\")"
         "  (literal \"/dev/zero\"))"]

        (map #(allow-rule ["file-read*"] %) read-paths)
        (map #(allow-rule ["file-read*" "file-map-executable"] %) executable-paths)
        (map #(allow-rule ["process-exec"] %) executable-paths)
        (map #(allow-rule ["file-read*" "file-write*"] %) writable-paths)
        (map #(allow-rule ["file-read*" "file-map-executable"] %) scratch-paths)
        (map #(allow-rule ["process-exec"] %) scratch-paths)

        ["(deny file-write-setugid)"
         (if network?
           "(allow network*)"
           "(deny network*)")]))))

(defn sandbox-exec
  "Build a `sandbox-exec` command prefix for the given sandbox options.

   The profile is passed as one argv element instead of through a temporary
   file. This avoids a profile replacement race between process creation and
   profile loading. Paths are escaped before being embedded in the profile."
  ([cmds]
   (sandbox-exec (which-sandbox-exec) cmds))
  ([executable cmds]
   (when-not executable
     (throw
       (IllegalArgumentException.
         "No 'sandbox-exec' executable is available.")))
   [(str executable) "-p" (profile cmds)]))
