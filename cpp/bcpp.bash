# bcpp(1) completion

_bcpp() {
   local cur prev words cword split
   _init_completion -s || return
   
   case $prev in
   -o)
      _filedir
      return
      ;;
   -I)
      filedir -d
      return
      ;;
   -D*|-U*)
      return
      ;;
   esac
   $split && return

   if [[ $cur == -D* ]]; then
      COMPREPLY=(-D)
      return
   elif [[ $cur == -U* ]]; then
      COMPREPLY=(-U)
      return
   elif [[ $cur == -* ]]; then
      COMPREPLY=($(compgen -W '$(_parse_help "$1" -h)' -- "$cur"))
      [[ ${COMPREPLY-} == *= ]] && compopt -o nospace
      return
   fi
   _filedir
} && complete -F _bcpp bcpp
