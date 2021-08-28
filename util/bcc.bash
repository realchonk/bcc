# bcc(1) completion

_bcc() {
   local cur prev words cword split
   _init_completion -s || return
   
   case $prev in
   -o|-e)
      _filedir
      return
      ;;
   -I|-L)
      _filedir -d
      return
      ;;
   -D*|-U*|-l*)
      return
   esac
   $split && return

   if [[ $cur == -* ]]; then
      if [[ $cur == -O* ]]; then
         COMPREPLY=($(compgen -W '-O0 -O1 -O2 -O3' -- "$cur"))
         return
      elif [[ $cur == -m* ]]; then
         COMPREPLY=(-mhelp $(compgen -W '$(_parse_help "$1" -mhelp)' -- "$cur"))
         return
      elif [[ $cur == -D* ]]; then
         COMPREPLY=(-D)
         return
      elif [[ $cur == -U* ]]; then
         COMPREPLY=(-U)
         return
      fi
      COMPREPLY=($(compgen -W '$(_parse_help "$1" -h)' -- "$cur"))
      [[ ${COMPREPLY-} == *= ]] && compopt -o nospace
      return
   fi
   _filedir
} && complete -F _bcc bcc
