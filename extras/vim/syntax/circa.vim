" Vim syntax file
" Language:		Circa

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Literal values
syn match circaInteger	"\<0[xX]\x\+\%(_\x\+\)*\>"								display
syn match circaInteger	"\<\%(0[dD]\)\=\%(0\|[1-9]\d*\%(_\d\+\)*\)\>"						display
syn match circaInteger	"\<0[oO]\=\o\+\%(_\o\+\)*\>"								display
syn match circaInteger	"\<0[bB][01]\+\%(_[01]\+\)*\>"								display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\.\d\+\%(_\d\+\)*\>"					display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\%(\.\d\+\%(_\d\+\)*\)\=\%([eE][-+]\=\d\+\%(_\d\+\)*\)\>"	display
syn region circaString   start=+"+ end=+"+ skip=+\\"+
syn region circaString   start=+'+ end=+'+ skip=+\\'+

" Keywords
syn keyword circaKeyword def type end if else for state return in true false
syn match circaKeyword "\<do\ once\>"

" Comments
syn region circaLineComment start="--" skip="\\$" end="$"

hi def link circaInteger        Number
hi def link circaFloat          Number
hi def link circaString         String
hi def link circaKeyword        Statement
hi def link circaLineComment    Comment

let b:current_syntax = "circa"

" vim: nowrap sw=2 sts=2 ts=8 ff=unix:
