#!/usr/bin/ksh 
###################################################################
# Script: usu_impre.sh                                            #
# Autor: Ariel Lombardero                                         #
# Funcion: Consulta relacion usuario - impresora                  #
# Fecha: 22-01-2004                                               #
###################################################################
#
veri_server()
# Proceso de verificacion server con impresoras asignadas a usuarios
#
{
   central=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep CENTRAL|awk '{print $2}')
   if [[ $central != "no" ]] ; then
      rshserver=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep RSHSERVER|awk '{print $2}')
      host=$(hostname)
      if [[ $rshserver != $host ]] ; then
         echo "\nEsta opcion solo esta habilitada para una Planta, o para $rshserver !"
         sleep 4
         return 1
      fi
   fi
}
      
###################################################################
#
# CUERPO PRINCIPAL
#
veri_server
if [[ $? -eq 1 ]] ; then
   return
fi

# Consulta cuales son las impresoras asignadas al usuario.
echo " \nIngrese el Usuario para buscar la Impresora==>> \c"
read usuario
if [[ -n $usuario ]] ; then
   echo "\n"
   grep -i -w $usuario /usr/sistemas/usu_prn.tab
   if [[ $? -ne 0 ]] ; then
      echo "Este usuario no existe o no tiene impresora asignada !"
   fi
   echo "\nPresione una tecla  . . ."
   read
fi
   

