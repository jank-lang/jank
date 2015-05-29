" Language: Jank Lisp
" Maintainer: Jeaye <contact@jeaye.com>

" Quit when a (custom) syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

let b:current_syntax = "jank"

syntax keyword jank_todo TODO XXX FIXME NOTE
syntax keyword jank_keyword function ƒ
syntax region	jank_string start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=jank_todo extend
syntax region jank_comment start="(;" skipnl end=";)" contains=jank_comment,jank_todo
syntax match jank_boolean "true\|false"

highlight link jank_keyword Keyword
highlight link jank_string String
highlight link jank_comment Comment
highlight link jank_todo Todo
highlight link jank_boolean Underlined

let &cpo = s:cpo_save
unlet s:cpo_save
