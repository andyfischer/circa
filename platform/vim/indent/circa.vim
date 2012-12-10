" Vim indent file
" Language:	Circa

" Only load this indent file when no other was loaded.
if exists("b:did_indent")
   finish
endif
let b:did_indent = 1

setlocal autoindent
setlocal indentexpr=GetCircaIndent()

setlocal indentkeys+=0=end

" Only define the function once.
if exists("*GetCircaIndent")
  finish
endif

echo 'loading'

function! GetCircaIndent()
  " Find a non-blank line above the current line.
  let lnum = prevnonblank(v:lnum - 1)

  " Hit the start of the file, use zero indent.
  if lnum == 0
    return 0
  endif

  " Add a 'shiftwidth' after lines that start a block:
  " 'def', 'for', 'if', 'else'
  let ind = indent(lnum)
  let flag = 0
  let prevline = getline(lnum)
  if prevline =~ '^\s*\(begin\>\|if\>\|namespace\>\|for\>\|elif\>\|else\>\|def\>\|do\ once\>\)'
      let ind = ind + &shiftwidth
      let flag = 1
  endif

  " Subtract a 'shiftwidth' after lines ending with
  " 'end' when they begin with 'if', 'for', etc. too.
  if flag == 1 && prevline =~ '\<end\>'
    let ind = ind - &shiftwidth
  endif

  " Subtract a 'shiftwidth' on end, else (and elseif), until and '}'
  " This is the part that requires 'indentkeys'.
  if getline(v:lnum) =~ '^\s*\(end\|else\)'
    let ind = ind - &shiftwidth
  endif

  return ind
endfunction
