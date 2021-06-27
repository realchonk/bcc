# bcc(1) completion

_bcc() {
   local cur prev words cword split
   _init_completion -s || return
   
   case $prev in
   -o)
      _filedir
      return
      ;;
   esac
   $split && return

   if [[ $cur == -* ]]; then
      COMPREPLY=($(compgen -W '$(_parse_help "$1" -h)' -- "$cur"))
      [[ ${COMPREPLY-} == *= ]] && compopt -o nospace
      return
   fi
   _filedir
} && complete -F _bcc bcc
