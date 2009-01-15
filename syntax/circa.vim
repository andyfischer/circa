" Vim syntax file
" Language:		Circa
"
" This syntax is based on the ruby.vim syntax

" Expression Substitution and Backslash Notation
syn match circaEscape		"\\\\\|\\[abefnrstv]\|\\\o\{1,3}\|\\x\x\{1,2}"								contained display
syn match circaEscape		"\%(\\M-\\C-\|\\C-\\M-\|\\M-\\c\|\\c\\M-\|\\c\|\\C-\|\\M-\)\%(\\\o\{1,3}\|\\x\x\{1,2}\|\\\=\S\)"	contained display
syn match circaInterpolation	"#{[^}]*}"				contained
syn match circaInterpolation	"#\%(\$\|@@\=\)\w\+"			contained display
syn match circaNoInterpolation	"\\#{[^}]*}"				contained
syn match circaNoInterpolation	"\\#\%(\$\|@@\=\)\w\+"			contained display

syn match circaDelimEscape	"\\[(<{\[)>}\]]" transparent display contained contains=NONE

syn region circaNestedParentheses	start="("	end=")"		skip="\\\\\|\\)"	transparent contained contains=@circaStringSpecial,circaNestedParentheses,circaDelimEscape
syn region circaNestedCurlyBraces	start="{"	end="}"		skip="\\\\\|\\}"	transparent contained contains=@circaStringSpecial,circaNestedCurlyBraces,circaDelimEscape
syn region circaNestedAngleBrackets	start="<"	end=">"		skip="\\\\\|\\>"	transparent contained contains=@circaStringSpecial,circaNestedAngleBrackets,circaDelimEscape
syn region circaNestedSquareBrackets	start="\["	end="\]"	skip="\\\\\|\\\]"	transparent contained contains=@circaStringSpecial,circaNestedSquareBrackets,circaDelimEscape

syn cluster circaStringSpecial		contains=circaInterpolation,circaNoInterpolation,circaEscape
syn cluster circaExtendedStringSpecial	contains=@circaStringSpecial,circaNestedParentheses,circaNestedCurlyBraces,circaNestedAngleBrackets,circaNestedSquareBrackets

" Numbers and ASCII Codes
syn match circaASCIICode	"\w\@<!\%(?\%(\\M-\\C-\|\\C-\\M-\|\\M-\\c\|\\c\\M-\|\\c\|\\C-\|\\M-\)\=\%(\\\o\{1,3}\|\\x\x\{1,2}\|\\\=\S\)\)"
syn match circaInteger	"\<0[xX]\x\+\%(_\x\+\)*\>"								display
syn match circaInteger	"\<\%(0[dD]\)\=\%(0\|[1-9]\d*\%(_\d\+\)*\)\>"						display
syn match circaInteger	"\<0[oO]\=\o\+\%(_\o\+\)*\>"								display
syn match circaInteger	"\<0[bB][01]\+\%(_[01]\+\)*\>"								display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\.\d\+\%(_\d\+\)*\>"					display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\%(\.\d\+\%(_\d\+\)*\)\=\%([eE][-+]\=\d\+\%(_\d\+\)*\)\>"	display

" Identifiers
syn match circaLocalVariableOrMethod "\<[_[:lower:]][_-[:alnum:]]*" contains=NONE display transparent
syn match circaBlockArgument	    "&[_[:lower:]][_[:alnum:]]"		 contains=NONE display transparent

"syn match  circaConstant			"\%(\%(\.\@<!\.\)\@<!\<\|::\)\_s*\zs\u\w*\>\%(\s*(\)\@!"
syn match  circaClassVariable		"@@\h\w*" display
syn match  circaInstanceVariable		"@\h\w*"  display
syn match  circaGlobalVariable		"$\%(\h\w*\|-.\)"
syn match  circaSymbol			":\@<!:\%(\^\|\~\|<<\|<=>\|<=\|<\|===\|==\|=\~\|>>\|>=\|>\||\|-@\|-\|/\|\[]=\|\[]\|\*\*\|\*\|&\|%\|+@\|+\|`\)"
syn match  circaSymbol			":\@<!:\$\%(-.\|[`~<=>_,;:!?/.'"@$*\&+0]\)"
syn match  circaSymbol			":\@<!:\%(\$\|@@\=\)\=\h\w*[?!=]\="
syn region circaSymbol			start=":\@<!:\"" end="\"" skip="\\\\\|\\\""
syn match  circaBlockParameter		"\%(\%(\<do\>\|{\)\s*\)\@<=|\s*\zs[( ,a-zA-Z0-9_*)]\+\ze\s*|" display

syn match circaPredefinedVariable #$[!$&"'*+,./0:;<=>?@\`~1-9]#
syn match circaPredefinedVariable "$_\>"											   display
syn match circaPredefinedVariable "$-[0FIKadilpvw]\>"									   display
syn match circaPredefinedVariable "$\%(deferr\|defout\|stderr\|stdin\|stdout\)\>"					   display
syn match circaPredefinedVariable "$\%(DEBUG\|FILENAME\|KCODE\|LOADED_FEATURES\|LOAD_PATH\|PROGRAM_NAME\|SAFE\|VERBOSE\)\>" display
syn match circaPredefinedConstant "\%(\%(\.\@<!\.\)\@<!\|::\)\_s*\zs\%(MatchingData\|ARGF\|ARGV\|ENV\)\>\%(\s*(\)\@!"
syn match circaPredefinedConstant "\%(\%(\.\@<!\.\)\@<!\|::\)\_s*\zs\%(DATA\|FALSE\|NIL\|RUBY_PLATFORM\|RUBY_RELEASE_DATE\)\>\%(\s*(\)\@!"
syn match circaPredefinedConstant "\%(\%(\.\@<!\.\)\@<!\|::\)\_s*\zs\%(RUBY_VERSION\|STDERR\|STDIN\|STDOUT\|TOPLEVEL_BINDING\|TRUE\)\>\%(\s*(\)\@!"

" Normal Regular Expression
syn region circaString matchgroup=circaStringDelimiter start="\%(\%(^\|\<\%(and\|or\|while\|until\|unless\|if\|elsif\|when\|not\|then\)\|[;\~=!|&(,[>]\)\s*\)\@<=/" end="/[iomx]*" skip="\\\\\|\\/" contains=@circaStringSpecial
syn region circaString matchgroup=circaStringDelimiter start="\%(\<\%(split\|scan\|gsub\|sub\)\s*\)\@<=/" end="/[iomx]*" skip="\\\\\|\\/" contains=@circaStringSpecial

" Normal String and Shell Command Output
syn region circaString matchgroup=circaStringDelimiter start="\"" end="\"" skip="\\\\\|\\\"" contains=@circaStringSpecial
syn region circaString matchgroup=circaStringDelimiter start="'"	end="'"  skip="\\\\\|\\'"
syn region circaString matchgroup=circaStringDelimiter start="`"	end="`"  skip="\\\\\|\\`"  contains=@circaStringSpecial

" Generalized Regular Expression
syn region circaString matchgroup=circaStringDelimiter start="%r\z([~`!@#$%^&*_\-+=|\:;"',.?/]\)"	end="\z1[iomx]*" skip="\\\\\|\\\z1" contains=@circaStringSpecial fold
syn region circaString matchgroup=circaStringDelimiter start="%r{"				end="}[iomx]*"	 skip="\\\\\|\\}"   contains=@circaStringSpecial,circaNestedCurlyBraces,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%r<"				end=">[iomx]*"	 skip="\\\\\|\\>"   contains=@circaStringSpecial,circaNestedAngleBrackets,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%r\["				end="\][iomx]*"	 skip="\\\\\|\\\]"  contains=@circaStringSpecial,circaNestedSquareBrackets,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%r("				end=")[iomx]*"	 skip="\\\\\|\\)"   contains=@circaStringSpecial,circaNestedParentheses,circaDelimEscape fold

" Generalized Single Quoted String, Symbol and Array of Strings
syn region circaString matchgroup=circaStringDelimiter start="%[qsw]\z([~`!@#$%^&*_\-+=|\:;"',.?/]\)" end="\z1" skip="\\\\\|\\\z1" fold
syn region circaString matchgroup=circaStringDelimiter start="%[qsw]{"				    end="}"   skip="\\\\\|\\}"	 fold	contains=circaNestedCurlyBraces,circaDelimEscape
syn region circaString matchgroup=circaStringDelimiter start="%[qsw]<"				    end=">"   skip="\\\\\|\\>"	 fold	contains=circaNestedAngleBrackets,circaDelimEscape
syn region circaString matchgroup=circaStringDelimiter start="%[qsw]\["				    end="\]"  skip="\\\\\|\\\]"	 fold	contains=circaNestedSquareBrackets,circaDelimEscape
syn region circaString matchgroup=circaStringDelimiter start="%[qsw]("				    end=")"   skip="\\\\\|\\)"	 fold	contains=circaNestedParentheses,circaDelimEscape

" Generalized Double Quoted String and Array of Strings and Shell Command Output
" Note: %= is not matched here as the beginning of a double quoted string
syn region circaString matchgroup=circaStringDelimiter start="%\z([~`!@#$%^&*_\-+|\:;"',.?/]\)"	    end="\z1" skip="\\\\\|\\\z1" contains=@circaStringSpecial fold
syn region circaString matchgroup=circaStringDelimiter start="%[QWx]\z([~`!@#$%^&*_\-+=|\:;"',.?/]\)" end="\z1" skip="\\\\\|\\\z1" contains=@circaStringSpecial fold
syn region circaString matchgroup=circaStringDelimiter start="%[QWx]\={"				    end="}"   skip="\\\\\|\\}"	 contains=@circaStringSpecial,circaNestedCurlyBraces,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%[QWx]\=<"				    end=">"   skip="\\\\\|\\>"	 contains=@circaStringSpecial,circaNestedAngleBrackets,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%[QWx]\=\["				    end="\]"  skip="\\\\\|\\\]"	 contains=@circaStringSpecial,circaNestedSquareBrackets,circaDelimEscape fold
syn region circaString matchgroup=circaStringDelimiter start="%[QWx]\=("				    end=")"   skip="\\\\\|\\)"	 contains=@circaStringSpecial,circaNestedParentheses,circaDelimEscape fold

" Here Document
syn region circaHeredocStart matchgroup=circaStringDelimiter start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-\=\zs\%(\h\w*\)+   end=+$+ oneline contains=TOP
syn region circaHeredocStart matchgroup=circaStringDelimiter start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-\=\zs"\%([^"]*\)"+ end=+$+ oneline contains=TOP
syn region circaHeredocStart matchgroup=circaStringDelimiter start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-\=\zs'\%([^']*\)'+ end=+$+ oneline contains=TOP
syn region circaHeredocStart matchgroup=circaStringDelimiter start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-\=\zs`\%([^`]*\)`+ end=+$+ oneline contains=TOP

syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<\z(\h\w*\)\ze+hs=s+2    matchgroup=circaStringDelimiter end=+^\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<"\z([^"]*\)"\ze+hs=s+2  matchgroup=circaStringDelimiter end=+^\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<'\z([^']*\)'\ze+hs=s+2  matchgroup=circaStringDelimiter end=+^\z1$+ contains=circaHeredocStart		      nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<`\z([^`]*\)`\ze+hs=s+2  matchgroup=circaStringDelimiter end=+^\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend

syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-\z(\h\w*\)\ze+hs=s+3    matchgroup=circaStringDelimiter end=+^\s*\zs\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-"\z([^"]*\)"\ze+hs=s+3  matchgroup=circaStringDelimiter end=+^\s*\zs\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-'\z([^']*\)'\ze+hs=s+3  matchgroup=circaStringDelimiter end=+^\s*\zs\z1$+ contains=circaHeredocStart		     nextgroup=circaFunction fold keepend
syn region circaString start=+\%(\%(class\s*\|\%(\.\|::\)\)\_s*\)\@<!<<-`\z([^`]*\)`\ze+hs=s+3  matchgroup=circaStringDelimiter end=+^\s*\zs\z1$+ contains=circaHeredocStart,@circaStringSpecial nextgroup=circaFunction fold keepend

if exists('main_syntax') && main_syntax == 'ecirca'
  let b:circa_no_expensive = 1
end

" Expensive Mode - colorize *end* according to opening statement
if !exists("b:circa_no_expensive") && !exists("circa_no_expensive")
  syn region circaFunction matchgroup=circaDefine start="\<def\s\+"    end="\%(\s*\%(\s\|(\|;\|$\|#\)\)\@=" oneline
  syn region circaClass	  matchgroup=circaDefine start="\<class\s\+"  end="\%(\s*\%(\s\|<\|;\|$\|#\)\)\@=" oneline
  syn match  circaDefine   "\<class\ze<<"
  syn region circaModule   matchgroup=circaDefine start="\<module\s\+" end="\%(\s*\%(\s\|;\|$\|#\)\)\@="	  oneline

  syn region circaBlock start="\<def\>"	  matchgroup=circaDefine end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo nextgroup=circaFunction fold
  syn region circaBlock start="\<class\>"  matchgroup=circaDefine end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo nextgroup=circaClass	 fold
  syn region circaBlock start="\<module\>" matchgroup=circaDefine end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo nextgroup=circaModule	 fold

  " modifiers
  syn match  circaControl "\<\%(if\|while\|)\>" display

  " *do* requiring *end*
  syn region circaDoBlock matchgroup=circaControl start="\<do\>" end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo fold

  " *{* requiring *}*
  syn region circaCurlyBlock start="{" end="}" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo fold

  " statements without *do*
  syn region circaNoDoBlock matchgroup=circaControl start="\<\%(case\|begin\)\>" start="\%(^\|\.\.\.\=\|[,;=([<>~\*/%!&^|+-]\)\s*\zs\%(if\|unless\)\>" end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo fold

  " statement with optional *do*
  syn region circaOptDoLine matchgroup=circaControl start="\<for\>" start="\%(\%(^\|\.\.\.\=\|[?:,;=([<>~\*/%!&^|+-]\|\%(\<[_[:lower:]][_[:alnum:]]*\)\@<![!=?]\)\s*\)\@<=\<\%(until\|while\)\>" end="\%(\<do\>\|:\)" end="\ze\%(;\|$\)" oneline contains=ALLBUT,@circaExtendedStringSpecial,circaTodo
  syn region circaOptDoBlock start="\<for\>" start="\%(\%(^\|\.\.\.\=\|[:,;([<>~\*/%&^|+-]\|\%(\<[_[:lower:]][_[:alnum:]]*\)\@<![!=?]\)\s*\)\@<=\<\%(until\|while\)\>" matchgroup=circaControl end="\<end\>" contains=ALLBUT,@circaExtendedStringSpecial,circaTodo nextgroup=circaOptDoLine fold

  if !exists("circa_minlines")
    let circa_minlines = 50
  endif
  exec "syn sync minlines=" . circa_minlines

else
  syn region  circaFunction matchgroup=circaControl start="\<def\s\+"    end="\ze\%(\s\|(\|;\|$\)" oneline
  syn region  circaClass    matchgroup=circaControl start="\<class\s\+"  end="\ze\%(\s\|<\|;\|$\)" oneline
  syn match   circaControl  "\<class\ze<<"
  syn region  circaModule   matchgroup=circaControl start="\<module\s\+" end="\ze\%(\s\|;\|$\)"	 oneline
  syn keyword circaControl case begin do for if unless while until end
endif

" Keywords
" Note: the following keywords have already been defined:
" begin case class def do end for if module unless until while
syn keyword circaControl		and break else elsif ensure in next not or redo rescue retry return then when
syn match   circaOperator	"\<defined?" display
syn keyword circaKeyword		alias super undef yield
syn keyword circaBoolean		true false
syn keyword circaPseudoVariable	nil self __FILE__ __LINE__
syn keyword circaBeginEnd	BEGIN END

" Comments and Documentation
syn match   circaSharpBang     "\%^#!.*" display
syn keyword circaTodo	      FIXME NOTE TODO XXX contained
syn match   circaComment       "--.*" contains=circaSharpBang,circaSpaceError,circaTodo,@Spell
syn region  circaDocumentation start="^=begin" end="^=end.*$" contains=circaSpaceError,circaTodo,@Spell fold

" Note: this is a hack to prevent 'keywords' being highlighted as such when called as methods with an explicit receiver
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(alias\|and\|begin\|break\|case\|class\|def\|defined\|do\|else\)\>"			transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(elsif\|end\|ensure\|false\|for\|if\|in\|module\|next\|nil\)\>"			transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(not\|or\|redo\|rescue\|retry\|return\|self\|super\|then\|true\)\>"			transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(undef\|unless\|until\|when\|while\|yield\|BEGIN\|END\|__FILE__\|__LINE__\)\>"	transparent contains=NONE

syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(abort\|at_exit\|attr\|attr_accessor\|attr_reader\)\>"	transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(attr_writer\|autoload\|callcc\|catch\|caller\)\>"		transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(eval\|class_eval\|instance_eval\|module_eval\|exit\)\>"	transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(extend\|fail\|fork\|include\|lambda\)\>"			transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(load\|loop\|private\|proc\|protected\)\>"			transparent contains=NONE
syn match circaKeywordAsMethod "\%(\%(\.\@<!\.\)\|::\)\_s*\%(public\|require\|raise\|throw\|trap\)\>"			transparent contains=NONE

" __END__ Directive
syn region circaData matchgroup=circaDataDirective start="^__END__$" end="\%$" fold

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_circa_syntax_inits")
  if version < 508
    let did_circa_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink circaDefine			Define
  HiLink circaFunction			Function
  HiLink circaControl			Statement
  HiLink circaInclude			Include
  HiLink circaInteger			Number
  HiLink circaASCIICode			circaInteger
  HiLink circaFloat			Float
  HiLink circaBoolean			circaPseudoVariable
  HiLink circaException			Exception
  HiLink circaClass			Type
  HiLink circaModule			Type
  if !exists("circa_no_identifiers")
    HiLink circaIdentifier		Identifier
  else
    HiLink circaIdentifier		NONE
  endif
  HiLink circaClassVariable		circaIdentifier
  HiLink circaConstant			circaIdentifier
  HiLink circaGlobalVariable		circaIdentifier
  HiLink circaBlockParameter		circaIdentifier
  HiLink circaInstanceVariable		circaIdentifier
  HiLink circaPredefinedIdentifier	circaIdentifier
  HiLink circaPredefinedConstant		circaPredefinedIdentifier
  HiLink circaPredefinedVariable		circaPredefinedIdentifier
  HiLink circaSymbol			circaIdentifier
  HiLink circaKeyword			Keyword
  HiLink circaOperator			Operator
  HiLink circaBeginEnd			Statement
  HiLink circaAccess			Statement
  HiLink circaAttribute			Statement
  HiLink circaEval			Statement
  HiLink circaPseudoVariable		Constant

  HiLink circaComment			Comment
  HiLink circaData			Comment
  HiLink circaDataDirective		Delimiter
  HiLink circaDocumentation		Comment
  HiLink circaEscape			Special
  HiLink circaInterpolation		Special
  HiLink circaNoInterpolation		circaString
  HiLink circaSharpBang			PreProc
  HiLink circaStringDelimiter		Delimiter
  HiLink circaString			String
  HiLink circaTodo			Todo

  HiLink circaError			Error
  HiLink circaSpaceError			circaError

  delcommand HiLink
endif

let b:current_syntax = "circa"

" vim: nowrap sw=2 sts=2 ts=8 ff=unix:
