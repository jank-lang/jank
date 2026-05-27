(ns leiningen.jank.sandbox.bwrap
  (:require [babashka.process :as proc]
            [babashka.fs :as fs]))

;; TODO: consider the merits of an allowlist vs. denylist of mounts. Per the
;; bwrap documentation, we definitely want to avoid mounting /var which may
;; contain the Docker or dbus sockets allowing privilege escalation.
;;
;; An allowlist avoids this issue, but if some OS uses non-standard mount points
;; (for example /nix is required on NixOS) then the container may not be fully
;; functional.
;;
;; Is a blanket mount of / with a denylist including /var safe?
(def standard-binds
  "Directories which are required for typical program execution.

  Some of these may not exist on all systems."
  [[:ro-bind "/usr" "/usr"]
   [:ro-bind "/bin" "/bin"]
   [:ro-bind "/lib" "/lib"]
   [:ro-bind "/lib64" "/lib64"]
   [:ro-bind "/etc" "/etc"]
   [:ro-bind "/nix" "/nix"]])

(defn which-bwrap
  "Find the `bwrap` executable on the path, or nil if it cannot be found."
  []
  (fs/which "bwrap"))

(defn bind-valid? [src dst]
  (fs/exists? src))

(defn bwrap
  [cmds]
  (concat
   ["bwrap"
    ;; Without this bwrap processes could outlive this process which spawned it.
    "--die-with-parent"
    ;; Prevent the child process from injecting keystrokes into the parent
    ;; terminal.
    "--new-session"
    ;; Start with a clean slate and add namespaces back via '--share-*'
    ;; commands.
    "--unshare-all"]
   (->> (for [[k & rst] cmds]
          (case k
            :ro-bind (let [[src dst] rst] (when (bind-valid? src dst) ["--ro-bind" src dst]))
            :bind    (let [[src dst] rst] (when (bind-valid? src dst) ["--bind" src dst]))
            :tmpfs   (let [dir rst] ["--tmpfs" dir])
            :chdir   (let [dir rst] ["--chdir" dir])
            :net     (let [share rst] (when share ["--share-net"]))))
        (flatten)
        (remove nil?))
   ["--proc" "/proc"
    "--dev" "/dev"]))

(comment
  (bwrap
   [[:ro-bind "/src" "/dest"]
    [:tmpfs "/tmp"]
    [:net true]]))
