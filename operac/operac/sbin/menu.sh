###################################################################
# Script: menu.sh						  #
# Autor: Ariel Lombardero                                         #
# Funcion: Manejador de menues                                    #
# Fecha: 7-11-2002                                                #
###################################################################
#
# Defino variables
titulo=`head -n 1 $1`
tot_lin=`wc -l $1 | awk '{ print $1 }'`
let opciones=tot_lin-1
#
# Guardo opciones 
lin=0
for i in `tail -n $opciones $1 | sed "s/ /:/g"`
do
	let lin=lin+1
	menu[$lin]=`echo $i | awk -F~ '{ print $1 }' | sed "s/:/ /g"`
	proc[$lin]=`echo $i | awk -F~ '{ print $2 }' | sed "s/:/ /g"`
done
#
# Mostrar por pantalla y elegir opcion
i_elije=true
while $i_elije 
do
	clear
	#
	# Imprimo titulo
	nombre=`cat /etc/passwd | grep $LOGIN | awk -F\: '{print $5}'`
	fecha=`date "+%d/%m/%y"`
	echo
	echo "      Usuario: $LOGIN  ($nombre)\c"
	echo "            $fecha"
	echo "      ___________________________________________________________"
	echo ""
	echo "                       $titulo"
	echo "      ___________________________________________________________"
	echo "  			   		     PETROBRAS ENERGIA "
	echo "      -----------------------------------------------------------"
	lin=0
	until [ lin -eq $opciones ] 
	do
		let lin=lin+1
		if [ $lin -lt 10 ] ; then
			echo "       $lin. ${menu[$lin]}" 2>/dev/null
		else
			echo "      $lin. ${menu[$lin]}" 2>/dev/null
		fi
	done
	echo
	echo "      -----------------------------------------------------------"
	echo "       x. Salir"
	echo "      ___________________________________________________________"
	echo "      Opcion: \c"
	read opcion 2>/dev/null
	if [[ -n $opcion && ( $opcion = "x" || $opcion = "X" ) ]] ; then
		i_elije=false
        else
            let numero=opcion+1 2>/dev/null
            if [[ $? = 0 ]] ; then
	       if [[ $opcion -gt 0 && $opcion -lt $tot_lin ]] ; then
		   proceso=${proc[$opcion]}
	           if [[ -n $proceso ]] ; then
		      echo $proceso | grep "menu.sh" >/dev/null
		      if [[ $? = 0 ]] ; then
	                 trap "mensajes.sh 2 1" 2
		         eval $proceso 2>/dev/null
	                 trap "" 2
		      else
		         trap 2 15
		         eval $proceso 2>/dev/null
		         trap "" 2 15
		      fi
		      if [[ $? -ne 0 ]] ; then
			      mensajes.sh 2 1
		      fi
	           else
		      mensajes.sh 3 1
	           fi
	       else
		   mensajes.sh 1 1
               fi
            else
	       if [[ -n $opcion ]] ; then
                  mensajes.sh 1 1
	       fi
            fi
	fi
done
clear
