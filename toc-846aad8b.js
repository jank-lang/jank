// Populate the sidebar
//
// This is a script, and not included directly in the page, to control the total size of the book.
// The TOC contains an entry for each page, so if each page includes a copy of the TOC,
// the total size of the page becomes O(n**2).
class MDBookSidebarScrollbox extends HTMLElement {
    constructor() {
        super();
    }
    connectedCallback() {
        this.innerHTML = '<ol class="chapter"><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="index.html">Welcome to the jank alpha!</a></span></li><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="foreword.html">Foreword</a></span></li><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="getting-started/index.html"><strong aria-hidden="true">1.</strong> Getting Started</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="getting-started/01-installation.html"><strong aria-hidden="true">1.1.</strong> Installation</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="getting-started/02-hello-world.html"><strong aria-hidden="true">1.2.</strong> Hello, world!</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="getting-started/03-hello-leiningen.html"><strong aria-hidden="true">1.3.</strong> Hello, Leiningen!</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="getting-started/04-hello-nrepl.html"><strong aria-hidden="true">1.4.</strong> Hello, nREPL!</a></span></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="cpp-interop/index.html"><strong aria-hidden="true">2.</strong> Reaching into C++</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/native-values.html"><strong aria-hidden="true">2.1.</strong> Working with native values</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/native-types.html"><strong aria-hidden="true">2.2.</strong> Working with native types</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/native-functions.html"><strong aria-hidden="true">2.3.</strong> Working with native functions</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/dsl.html"><strong aria-hidden="true">2.4.</strong> The C++ DSL</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/native-exceptions.html"><strong aria-hidden="true">2.5.</strong> Throwing and catching exceptions</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/cast.html"><strong aria-hidden="true">2.6.</strong> Casting between native types</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/cpp-raw.html"><strong aria-hidden="true">2.7.</strong> Embedding raw C++</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="cpp-interop/cpp-ns.html"><strong aria-hidden="true">2.8.</strong> The cpp namespace</a></span></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="project/index.html"><strong aria-hidden="true">3.</strong> Working with projects</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="project/test.html"><strong aria-hidden="true">3.1.</strong> Testing</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="project/aot.html"><strong aria-hidden="true">3.2.</strong> AOT compiling</a></span></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="jank-build/index.html"><strong aria-hidden="true">4.</strong> The jank build system</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="jank-build/overview.html"><strong aria-hidden="true">4.1.</strong> Build system overview</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="jank-build/build-cache.html"><strong aria-hidden="true">4.2.</strong> The build cache</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="jank-build/packaging-system-lib.html"><strong aria-hidden="true">4.3.</strong> Guide: Packaging a system library</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="jank-build/packaging-source-lib.html"><strong aria-hidden="true">4.4.</strong> Guide: Packaging a source library</a></span></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="differences-from-clojure.html"><strong aria-hidden="true">5.</strong> Differences from Clojure</a></span></li><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="troubleshooting/index.html"><strong aria-hidden="true">6.</strong> Troubleshooting</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="troubleshooting/health-check.html"><strong aria-hidden="true">6.1.</strong> Checking jank&#39;s health</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="troubleshooting/printing.html"><strong aria-hidden="true">6.2.</strong> Printing jank&#39;s IR or codegen</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="troubleshooting/stack-trace.html"><strong aria-hidden="true">6.3.</strong> How to get a stack trace</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="troubleshooting/getting-help.html"><strong aria-hidden="true">6.4.</strong> Where to get help</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="troubleshooting/faq.html"><strong aria-hidden="true">6.5.</strong> FAQ</a></span></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="reference/index.html"><strong aria-hidden="true">7.</strong> Reference</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error.html"><strong aria-hidden="true">7.1.</strong> Errors</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/lex/index.html"><strong aria-hidden="true">7.1.1.</strong> Lex</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/unexpected-eof.html"><strong aria-hidden="true">7.1.1.1.</strong> lex/unexpected-eof</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/expecting-whitespace.html"><strong aria-hidden="true">7.1.1.2.</strong> lex/expecting-whitespace</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/invalid-unicode.html"><strong aria-hidden="true">7.1.1.3.</strong> lex/invalid-unicode</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/incomplete-character.html"><strong aria-hidden="true">7.1.1.4.</strong> lex/incomplete-character</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/invalid-number.html"><strong aria-hidden="true">7.1.1.5.</strong> lex/invalid-number</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/invalid-ratio.html"><strong aria-hidden="true">7.1.1.6.</strong> lex/invalid-ratio</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/invalid-symbol.html"><strong aria-hidden="true">7.1.1.7.</strong> lex/invalid-symbol</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/invalid-keyword.html"><strong aria-hidden="true">7.1.1.8.</strong> lex/invalid-keyword</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/unterminated-string.html"><strong aria-hidden="true">7.1.1.9.</strong> lex/unterminated-string</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/unexpected-character.html"><strong aria-hidden="true">7.1.1.10.</strong> lex/unexpected-character</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/lex/internal-failure.html"><strong aria-hidden="true">7.1.1.11.</strong> lex/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/parse/index.html"><strong aria-hidden="true">7.1.2.</strong> Parse</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-unicode.html"><strong aria-hidden="true">7.1.2.1.</strong> parse/invalid-unicode</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-character.html"><strong aria-hidden="true">7.1.2.2.</strong> parse/invalid-character</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-string-escape.html"><strong aria-hidden="true">7.1.2.3.</strong> parse/invalid-string-escape</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unexpected-closing-character.html"><strong aria-hidden="true">7.1.2.4.</strong> parse/unexpected-closing-character</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unterminated-list.html"><strong aria-hidden="true">7.1.2.5.</strong> parse/unterminated-list</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unterminated-vector.html"><strong aria-hidden="true">7.1.2.6.</strong> parse/unterminated-vector</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unterminated-map.html"><strong aria-hidden="true">7.1.2.7.</strong> parse/unterminated-map</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unterminated-set.html"><strong aria-hidden="true">7.1.2.8.</strong> parse/unterminated-set</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/odd-entries-in-map.html"><strong aria-hidden="true">7.1.2.9.</strong> parse/odd-entries-in-map</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/duplicate-keys-in-map.html"><strong aria-hidden="true">7.1.2.10.</strong> parse/duplicate-keys-in-map</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/duplicate-items-in-set.html"><strong aria-hidden="true">7.1.2.11.</strong> parse/duplicate-items-in-set</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-quote.html"><strong aria-hidden="true">7.1.2.12.</strong> parse/invalid-quote</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-meta-hint-value.html"><strong aria-hidden="true">7.1.2.13.</strong> parse/invalid-meta-hint-value</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-meta-hint-target.html"><strong aria-hidden="true">7.1.2.14.</strong> parse/invalid-meta-hint-target</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/unsupported-reader-macro.html"><strong aria-hidden="true">7.1.2.15.</strong> parse/unsupported-reader-macro</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/nested-shorthand-function.html"><strong aria-hidden="true">7.1.2.16.</strong> parse/nested-shorthand-function</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-shorthand-function-parameter.html"><strong aria-hidden="true">7.1.2.17.</strong> parse/invalid-shorthand-function-parameter</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-var.html"><strong aria-hidden="true">7.1.2.18.</strong> parse/invalid-reader-var</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-comment.html"><strong aria-hidden="true">7.1.2.19.</strong> parse/invalid-reader-comment</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-conditional.html"><strong aria-hidden="true">7.1.2.20.</strong> parse/invalid-reader-conditional</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-splice.html"><strong aria-hidden="true">7.1.2.21.</strong> parse/invalid-reader-splice</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-gensym.html"><strong aria-hidden="true">7.1.2.22.</strong> parse/invalid-reader-gensym</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-symbolic-value.html"><strong aria-hidden="true">7.1.2.23.</strong> parse/invalid-reader-symbolic-value</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-tag-value.html"><strong aria-hidden="true">7.1.2.24.</strong> parse/invalid-reader-tag-value</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-regex.html"><strong aria-hidden="true">7.1.2.25.</strong> parse/invalid-regex</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-uuid.html"><strong aria-hidden="true">7.1.2.26.</strong> parse/invalid-uuid</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-inst.html"><strong aria-hidden="true">7.1.2.27.</strong> parse/invalid-inst</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-syntax-quote.html"><strong aria-hidden="true">7.1.2.28.</strong> parse/invalid-syntax-quote</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-syntax-unquote.html"><strong aria-hidden="true">7.1.2.29.</strong> parse/invalid-syntax-unquote</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-syntax-unquote-splice.html"><strong aria-hidden="true">7.1.2.30.</strong> parse/invalid-syntax-unquote-splice</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-reader-deref.html"><strong aria-hidden="true">7.1.2.31.</strong> parse/invalid-reader-deref</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-ratio.html"><strong aria-hidden="true">7.1.2.32.</strong> parse/invalid-ratio</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-keyword.html"><strong aria-hidden="true">7.1.2.33.</strong> parse/invalid-keyword</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/invalid-data-reader.html"><strong aria-hidden="true">7.1.2.34.</strong> parse/invalid-data-reader</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/parse/internal-failure.html"><strong aria-hidden="true">7.1.2.35.</strong> parse/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/analyze/index.html"><strong aria-hidden="true">7.1.3.</strong> Analyze</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-case.html"><strong aria-hidden="true">7.1.3.1.</strong> analyze/invalid-case</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-def.html"><strong aria-hidden="true">7.1.3.2.</strong> analyze/invalid-def</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-fn.html"><strong aria-hidden="true">7.1.3.3.</strong> analyze/invalid-fn</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-fn-parameters.html"><strong aria-hidden="true">7.1.3.4.</strong> analyze/invalid-fn-parameters</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-recur-position.html"><strong aria-hidden="true">7.1.3.5.</strong> analyze/invalid-recur-position</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-recur-from-try.html"><strong aria-hidden="true">7.1.3.6.</strong> analyze/invalid-recur-from-try</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-recur-args.html"><strong aria-hidden="true">7.1.3.7.</strong> analyze/invalid-recur-args</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-let.html"><strong aria-hidden="true">7.1.3.8.</strong> analyze/invalid-let</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-letfn.html"><strong aria-hidden="true">7.1.3.9.</strong> analyze/invalid-letfn</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-if.html"><strong aria-hidden="true">7.1.3.10.</strong> analyze/invalid-if</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-quote.html"><strong aria-hidden="true">7.1.3.11.</strong> analyze/invalid-quote</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-var-reference.html"><strong aria-hidden="true">7.1.3.12.</strong> analyze/invalid-var-reference</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/unresolved-var.html"><strong aria-hidden="true">7.1.3.13.</strong> analyze/unresolved-var</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/unresolved-symbol.html"><strong aria-hidden="true">7.1.3.14.</strong> analyze/unresolved-symbol</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/macro-expansion-exception.html"><strong aria-hidden="true">7.1.3.15.</strong> analyze/macro-expansion-exception</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-operator-call.html"><strong aria-hidden="true">7.1.3.16.</strong> analyze/invalid-cpp-operator-call</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-constructor-call.html"><strong aria-hidden="true">7.1.3.17.</strong> analyze/invalid-cpp-constructor-call</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-member-call.html"><strong aria-hidden="true">7.1.3.18.</strong> analyze/invalid-cpp-member-call</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-capture.html"><strong aria-hidden="true">7.1.3.19.</strong> analyze/invalid-cpp-capture</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-position.html"><strong aria-hidden="true">7.1.3.20.</strong> analyze/invalid-cpp-position</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/mismatched-if-types.html"><strong aria-hidden="true">7.1.3.21.</strong> analyze/mismatched-if-types</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-call.html"><strong aria-hidden="true">7.1.3.22.</strong> analyze/invalid-cpp-call</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-conversion.html"><strong aria-hidden="true">7.1.3.23.</strong> analyze/invalid-cpp-conversion</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-symbol.html"><strong aria-hidden="true">7.1.3.24.</strong> analyze/invalid-cpp-symbol</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/unresolved-cpp-symbol.html"><strong aria-hidden="true">7.1.3.25.</strong> analyze/unresolved-cpp-symbol</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-raw.html"><strong aria-hidden="true">7.1.3.26.</strong> analyze/invalid-cpp-raw</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-type.html"><strong aria-hidden="true">7.1.3.27.</strong> analyze/invalid-cpp-type</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-type-position.html"><strong aria-hidden="true">7.1.3.28.</strong> analyze/invalid-cpp-type-position</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-dsl.html"><strong aria-hidden="true">7.1.3.29.</strong> analyze/invalid-cpp-dsl</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-value.html"><strong aria-hidden="true">7.1.3.30.</strong> analyze/invalid-cpp-value</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-cast.html"><strong aria-hidden="true">7.1.3.31.</strong> analyze/invalid-cpp-cast</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-unsafe-cast.html"><strong aria-hidden="true">7.1.3.32.</strong> analyze/invalid-cpp-unsafe-cast</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-box.html"><strong aria-hidden="true">7.1.3.33.</strong> analyze/invalid-cpp-box</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-unbox.html"><strong aria-hidden="true">7.1.3.34.</strong> analyze/invalid-cpp-unbox</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-new.html"><strong aria-hidden="true">7.1.3.35.</strong> analyze/invalid-cpp-new</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-delete.html"><strong aria-hidden="true">7.1.3.36.</strong> analyze/invalid-cpp-delete</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/invalid-cpp-member-access.html"><strong aria-hidden="true">7.1.3.37.</strong> analyze/invalid-cpp-member-access</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/analyze/internal-failure.html"><strong aria-hidden="true">7.1.3.38.</strong> analyze/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/codegen/index.html"><strong aria-hidden="true">7.1.4.</strong> Codegen</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/codegen/internal-failure.html"><strong aria-hidden="true">7.1.4.1.</strong> codegen/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/aot/index.html"><strong aria-hidden="true">7.1.5.</strong> AOT</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/aot/unresolved-main.html"><strong aria-hidden="true">7.1.5.1.</strong> aot/unresolved-main</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/aot/internal-failure.html"><strong aria-hidden="true">7.1.5.2.</strong> aot/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/runtime/index.html"><strong aria-hidden="true">7.1.6.</strong> Runtime</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/module-not-found.html"><strong aria-hidden="true">7.1.6.1.</strong> runtime/module-not-found</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/module-binary-without-source.html"><strong aria-hidden="true">7.1.6.2.</strong> runtime/module-binary-without-source</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/unable-to-open-file.html"><strong aria-hidden="true">7.1.6.3.</strong> runtime/unable-to-open-file</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/invalid-cpp-eval.html"><strong aria-hidden="true">7.1.6.4.</strong> runtime/invalid-cpp-eval</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/unable-to-load-module.html"><strong aria-hidden="true">7.1.6.5.</strong> runtime/unable-to-load-module</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/invalid-unbox.html"><strong aria-hidden="true">7.1.6.6.</strong> runtime/invalid-unbox</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/non-metadatable-value.html"><strong aria-hidden="true">7.1.6.7.</strong> runtime/non-metadatable-value</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/invalid-referred-global-symbol.html"><strong aria-hidden="true">7.1.6.8.</strong> runtime/invalid-referred-global-symbol</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/invalid-referred-global-rename.html"><strong aria-hidden="true">7.1.6.9.</strong> runtime/invalid-referred-global-rename</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/unsupported-behavior.html"><strong aria-hidden="true">7.1.6.10.</strong> runtime/unsupported-behavior</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/static-feature-disabled.html"><strong aria-hidden="true">7.1.6.11.</strong> runtime/static-feature-disabled</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/uncaught-exception.html"><strong aria-hidden="true">7.1.6.12.</strong> runtime/uncaught-exception</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/runtime/internal-failure.html"><strong aria-hidden="true">7.1.6.13.</strong> runtime/internal-failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/system/index.html"><strong aria-hidden="true">7.1.7.</strong> System</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/system/clang-executable-not-found.html"><strong aria-hidden="true">7.1.7.1.</strong> system/clang-executable-not-found</a></span></li><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/system/failure.html"><strong aria-hidden="true">7.1.7.2.</strong> system/failure</a></span></li></ol><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/internal/index.html"><strong aria-hidden="true">7.1.8.</strong> Internal</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="reference/error/internal/failure.html"><strong aria-hidden="true">7.1.8.1.</strong> internal/failure</a></span></li></ol></li></ol></li></ol><li class="chapter-item expanded "><span class="chapter-link-wrapper"><a href="dev/index.html"><strong aria-hidden="true">8.</strong> Developing jank</a><a class="chapter-fold-toggle"><div>❱</div></a></span><ol class="section"><li class="chapter-item "><span class="chapter-link-wrapper"><a href="dev/ir.html"><strong aria-hidden="true">8.1.</strong> IR reference</a></span></li></ol></li></ol>';
        // Set the current, active page, and reveal it if it's hidden
        let current_page = document.location.href.toString().split('#')[0].split('?')[0];
        if (current_page.endsWith('/')) {
            current_page += 'index.html';
        }
        const links = Array.prototype.slice.call(this.querySelectorAll('a'));
        const l = links.length;
        for (let i = 0; i < l; ++i) {
            const link = links[i];
            const href = link.getAttribute('href');
            if (href && !href.startsWith('#') && !/^(?:[a-z+]+:)?\/\//.test(href)) {
                link.href = path_to_root + href;
            }
            // The 'index' page is supposed to alias the first chapter in the book.
            if (link.href === current_page
                || i === 0
                && path_to_root === ''
                && current_page.endsWith('/index.html')) {
                link.classList.add('active');
                let parent = link.parentElement;
                while (parent) {
                    if (parent.tagName === 'LI' && parent.classList.contains('chapter-item')) {
                        parent.classList.add('expanded');
                    }
                    parent = parent.parentElement;
                }
            }
        }
        // Track and set sidebar scroll position
        this.addEventListener('click', e => {
            if (e.target.tagName === 'A') {
                const clientRect = e.target.getBoundingClientRect();
                const sidebarRect = this.getBoundingClientRect();
                sessionStorage.setItem('sidebar-scroll-offset', clientRect.top - sidebarRect.top);
            }
        }, { passive: true });
        const sidebarScrollOffset = sessionStorage.getItem('sidebar-scroll-offset');
        sessionStorage.removeItem('sidebar-scroll-offset');
        if (sidebarScrollOffset !== null) {
            // preserve sidebar scroll position when navigating via links within sidebar
            const activeSection = this.querySelector('.active');
            if (activeSection) {
                const clientRect = activeSection.getBoundingClientRect();
                const sidebarRect = this.getBoundingClientRect();
                const currentOffset = clientRect.top - sidebarRect.top;
                this.scrollTop += currentOffset - parseFloat(sidebarScrollOffset);
            }
        } else {
            // scroll sidebar to current active section when navigating via
            // 'next/previous chapter' buttons
            const activeSection = document.querySelector('#mdbook-sidebar .active');
            if (activeSection) {
                activeSection.scrollIntoView({ block: 'center' });
            }
        }
        // Toggle buttons
        const sidebarAnchorToggles = document.querySelectorAll('.chapter-fold-toggle');
        function toggleSection(ev) {
            ev.currentTarget.parentElement.parentElement.classList.toggle('expanded');
        }
        Array.from(sidebarAnchorToggles).forEach(el => {
            el.addEventListener('click', toggleSection);
        });
    }
}
window.customElements.define('mdbook-sidebar-scrollbox', MDBookSidebarScrollbox);


// ---------------------------------------------------------------------------
// Support for dynamically adding headers to the sidebar.

(function() {
    // This is used to detect which direction the page has scrolled since the
    // last scroll event.
    let lastKnownScrollPosition = 0;
    // This is the threshold in px from the top of the screen where it will
    // consider a header the "current" header when scrolling down.
    const defaultDownThreshold = 150;
    // Same as defaultDownThreshold, except when scrolling up.
    const defaultUpThreshold = 300;
    // The threshold is a virtual horizontal line on the screen where it
    // considers the "current" header to be above the line. The threshold is
    // modified dynamically to handle headers that are near the bottom of the
    // screen, and to slightly offset the behavior when scrolling up vs down.
    let threshold = defaultDownThreshold;
    // This is used to disable updates while scrolling. This is needed when
    // clicking the header in the sidebar, which triggers a scroll event. It
    // is somewhat finicky to detect when the scroll has finished, so this
    // uses a relatively dumb system of disabling scroll updates for a short
    // time after the click.
    let disableScroll = false;
    // Array of header elements on the page.
    let headers;
    // Array of li elements that are initially collapsed headers in the sidebar.
    // I'm not sure why eslint seems to have a false positive here.
    // eslint-disable-next-line prefer-const
    let headerToggles = [];
    // This is a debugging tool for the threshold which you can enable in the console.
    let thresholdDebug = false;

    // Updates the threshold based on the scroll position.
    function updateThreshold() {
        const scrollTop = window.pageYOffset || document.documentElement.scrollTop;
        const windowHeight = window.innerHeight;
        const documentHeight = document.documentElement.scrollHeight;

        // The number of pixels below the viewport, at most documentHeight.
        // This is used to push the threshold down to the bottom of the page
        // as the user scrolls towards the bottom.
        const pixelsBelow = Math.max(0, documentHeight - (scrollTop + windowHeight));
        // The number of pixels above the viewport, at least defaultDownThreshold.
        // Similar to pixelsBelow, this is used to push the threshold back towards
        // the top when reaching the top of the page.
        const pixelsAbove = Math.max(0, defaultDownThreshold - scrollTop);
        // How much the threshold should be offset once it gets close to the
        // bottom of the page.
        const bottomAdd = Math.max(0, windowHeight - pixelsBelow - defaultDownThreshold);
        let adjustedBottomAdd = bottomAdd;

        // Adjusts bottomAdd for a small document. The calculation above
        // assumes the document is at least twice the windowheight in size. If
        // it is less than that, then bottomAdd needs to be shrunk
        // proportional to the difference in size.
        if (documentHeight < windowHeight * 2) {
            const maxPixelsBelow = documentHeight - windowHeight;
            const t = 1 - pixelsBelow / Math.max(1, maxPixelsBelow);
            const clamp = Math.max(0, Math.min(1, t));
            adjustedBottomAdd *= clamp;
        }

        let scrollingDown = true;
        if (scrollTop < lastKnownScrollPosition) {
            scrollingDown = false;
        }

        if (scrollingDown) {
            // When scrolling down, move the threshold up towards the default
            // downwards threshold position. If near the bottom of the page,
            // adjustedBottomAdd will offset the threshold towards the bottom
            // of the page.
            const amountScrolledDown = scrollTop - lastKnownScrollPosition;
            const adjustedDefault = defaultDownThreshold + adjustedBottomAdd;
            threshold = Math.max(adjustedDefault, threshold - amountScrolledDown);
        } else {
            // When scrolling up, move the threshold down towards the default
            // upwards threshold position. If near the bottom of the page,
            // quickly transition the threshold back up where it normally
            // belongs.
            const amountScrolledUp = lastKnownScrollPosition - scrollTop;
            const adjustedDefault = defaultUpThreshold - pixelsAbove
                + Math.max(0, adjustedBottomAdd - defaultDownThreshold);
            threshold = Math.min(adjustedDefault, threshold + amountScrolledUp);
        }

        if (documentHeight <= windowHeight) {
            threshold = 0;
        }

        if (thresholdDebug) {
            const id = 'mdbook-threshold-debug-data';
            let data = document.getElementById(id);
            if (data === null) {
                data = document.createElement('div');
                data.id = id;
                data.style.cssText = `
                    position: fixed;
                    top: 50px;
                    right: 10px;
                    background-color: 0xeeeeee;
                    z-index: 9999;
                    pointer-events: none;
                `;
                document.body.appendChild(data);
            }
            data.innerHTML = `
                <table>
                  <tr><td>documentHeight</td><td>${documentHeight.toFixed(1)}</td></tr>
                  <tr><td>windowHeight</td><td>${windowHeight.toFixed(1)}</td></tr>
                  <tr><td>scrollTop</td><td>${scrollTop.toFixed(1)}</td></tr>
                  <tr><td>pixelsAbove</td><td>${pixelsAbove.toFixed(1)}</td></tr>
                  <tr><td>pixelsBelow</td><td>${pixelsBelow.toFixed(1)}</td></tr>
                  <tr><td>bottomAdd</td><td>${bottomAdd.toFixed(1)}</td></tr>
                  <tr><td>adjustedBottomAdd</td><td>${adjustedBottomAdd.toFixed(1)}</td></tr>
                  <tr><td>scrollingDown</td><td>${scrollingDown}</td></tr>
                  <tr><td>threshold</td><td>${threshold.toFixed(1)}</td></tr>
                </table>
            `;
            drawDebugLine();
        }

        lastKnownScrollPosition = scrollTop;
    }

    function drawDebugLine() {
        if (!document.body) {
            return;
        }
        const id = 'mdbook-threshold-debug-line';
        const existingLine = document.getElementById(id);
        if (existingLine) {
            existingLine.remove();
        }
        const line = document.createElement('div');
        line.id = id;
        line.style.cssText = `
            position: fixed;
            top: ${threshold}px;
            left: 0;
            width: 100vw;
            height: 2px;
            background-color: red;
            z-index: 9999;
            pointer-events: none;
        `;
        document.body.appendChild(line);
    }

    function mdbookEnableThresholdDebug() {
        thresholdDebug = true;
        updateThreshold();
        drawDebugLine();
    }

    window.mdbookEnableThresholdDebug = mdbookEnableThresholdDebug;

    // Updates which headers in the sidebar should be expanded. If the current
    // header is inside a collapsed group, then it, and all its parents should
    // be expanded.
    function updateHeaderExpanded(currentA) {
        // Add expanded to all header-item li ancestors.
        let current = currentA.parentElement;
        while (current) {
            if (current.tagName === 'LI' && current.classList.contains('header-item')) {
                current.classList.add('expanded');
            }
            current = current.parentElement;
        }
    }

    // Updates which header is marked as the "current" header in the sidebar.
    // This is done with a virtual Y threshold, where headers at or below
    // that line will be considered the current one.
    function updateCurrentHeader() {
        if (!headers || !headers.length) {
            return;
        }

        // Reset the classes, which will be rebuilt below.
        const els = document.getElementsByClassName('current-header');
        for (const el of els) {
            el.classList.remove('current-header');
        }
        for (const toggle of headerToggles) {
            toggle.classList.remove('expanded');
        }

        // Find the last header that is above the threshold.
        let lastHeader = null;
        for (const header of headers) {
            const rect = header.getBoundingClientRect();
            if (rect.top <= threshold) {
                lastHeader = header;
            } else {
                break;
            }
        }
        if (lastHeader === null) {
            lastHeader = headers[0];
            const rect = lastHeader.getBoundingClientRect();
            const windowHeight = window.innerHeight;
            if (rect.top >= windowHeight) {
                return;
            }
        }

        // Get the anchor in the summary.
        const href = '#' + lastHeader.id;
        const a = [...document.querySelectorAll('.header-in-summary')]
            .find(element => element.getAttribute('href') === href);
        if (!a) {
            return;
        }

        a.classList.add('current-header');

        updateHeaderExpanded(a);
    }

    // Updates which header is "current" based on the threshold line.
    function reloadCurrentHeader() {
        if (disableScroll) {
            return;
        }
        updateThreshold();
        updateCurrentHeader();
    }


    // When clicking on a header in the sidebar, this adjusts the threshold so
    // that it is located next to the header. This is so that header becomes
    // "current".
    function headerThresholdClick(event) {
        // See disableScroll description why this is done.
        disableScroll = true;
        setTimeout(() => {
            disableScroll = false;
        }, 100);
        // requestAnimationFrame is used to delay the update of the "current"
        // header until after the scroll is done, and the header is in the new
        // position.
        requestAnimationFrame(() => {
            requestAnimationFrame(() => {
                // Closest is needed because if it has child elements like <code>.
                const a = event.target.closest('a');
                const href = a.getAttribute('href');
                const targetId = href.substring(1);
                const targetElement = document.getElementById(targetId);
                if (targetElement) {
                    threshold = targetElement.getBoundingClientRect().bottom;
                    updateCurrentHeader();
                }
            });
        });
    }

    // Takes the nodes from the given head and copies them over to the
    // destination, along with some filtering.
    function filterHeader(source, dest) {
        const clone = source.cloneNode(true);
        clone.querySelectorAll('mark').forEach(mark => {
            mark.replaceWith(...mark.childNodes);
        });
        dest.append(...clone.childNodes);
    }

    // Scans page for headers and adds them to the sidebar.
    document.addEventListener('DOMContentLoaded', function() {
        const activeSection = document.querySelector('#mdbook-sidebar .active');
        if (activeSection === null) {
            return;
        }

        const main = document.getElementsByTagName('main')[0];
        headers = Array.from(main.querySelectorAll('h2, h3, h4, h5, h6'))
            .filter(h => h.id !== '' && h.children.length && h.children[0].tagName === 'A');

        if (headers.length === 0) {
            return;
        }

        // Build a tree of headers in the sidebar.

        const stack = [];

        const firstLevel = parseInt(headers[0].tagName.charAt(1));
        for (let i = 1; i < firstLevel; i++) {
            const ol = document.createElement('ol');
            ol.classList.add('section');
            if (stack.length > 0) {
                stack[stack.length - 1].ol.appendChild(ol);
            }
            stack.push({level: i + 1, ol: ol});
        }

        // The level where it will start folding deeply nested headers.
        const foldLevel = 3;

        for (let i = 0; i < headers.length; i++) {
            const header = headers[i];
            const level = parseInt(header.tagName.charAt(1));

            const currentLevel = stack[stack.length - 1].level;
            if (level > currentLevel) {
                // Begin nesting to this level.
                for (let nextLevel = currentLevel + 1; nextLevel <= level; nextLevel++) {
                    const ol = document.createElement('ol');
                    ol.classList.add('section');
                    const last = stack[stack.length - 1];
                    const lastChild = last.ol.lastChild;
                    // Handle the case where jumping more than one nesting
                    // level, which doesn't have a list item to place this new
                    // list inside of.
                    if (lastChild) {
                        lastChild.appendChild(ol);
                    } else {
                        last.ol.appendChild(ol);
                    }
                    stack.push({level: nextLevel, ol: ol});
                }
            } else if (level < currentLevel) {
                while (stack.length > 1 && stack[stack.length - 1].level > level) {
                    stack.pop();
                }
            }

            const li = document.createElement('li');
            li.classList.add('header-item');
            li.classList.add('expanded');
            if (level < foldLevel) {
                li.classList.add('expanded');
            }
            const span = document.createElement('span');
            span.classList.add('chapter-link-wrapper');
            const a = document.createElement('a');
            span.appendChild(a);
            a.href = '#' + header.id;
            a.classList.add('header-in-summary');
            filterHeader(header.children[0], a);
            a.addEventListener('click', headerThresholdClick);
            const nextHeader = headers[i + 1];
            if (nextHeader !== undefined) {
                const nextLevel = parseInt(nextHeader.tagName.charAt(1));
                if (nextLevel > level && level >= foldLevel) {
                    const toggle = document.createElement('a');
                    toggle.classList.add('chapter-fold-toggle');
                    toggle.classList.add('header-toggle');
                    toggle.addEventListener('click', () => {
                        li.classList.toggle('expanded');
                    });
                    const toggleDiv = document.createElement('div');
                    toggleDiv.textContent = '❱';
                    toggle.appendChild(toggleDiv);
                    span.appendChild(toggle);
                    headerToggles.push(li);
                }
            }
            li.appendChild(span);

            const currentParent = stack[stack.length - 1];
            currentParent.ol.appendChild(li);
        }

        const onThisPage = document.createElement('div');
        onThisPage.classList.add('on-this-page');
        onThisPage.append(stack[0].ol);
        const activeItemSpan = activeSection.parentElement;
        activeItemSpan.after(onThisPage);
    });

    document.addEventListener('DOMContentLoaded', reloadCurrentHeader);
    document.addEventListener('scroll', reloadCurrentHeader, { passive: true });
})();

