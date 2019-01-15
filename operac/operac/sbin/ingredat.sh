#!/usr/bin/ksh 
###################################################################
# Script: ingredat.sh                                             #
# Autor: Ariel Lombardero                                         #
# Funcion: Ingreso de datos                                       #
# Fecha: 09-12-2003                                               #
###################################################################
#
# parametro 1: Texto previo al ingreso del dato.
# parametro 2: Valores validos del ingreso de los datos.
# parametro 3: Codigo de mensaje de error que debe dar.
# En caso de que el parametro 2 sea "Fecha", sólo se valida que lo ingresado
# sea mayor a uno. Si ingresa <ENTER>, le asigna la fecha del día, para lo
# cual, debe estar definida previamente la variable "$dia".
  m_texto=$1
  m_opciones=$2
  m_menserr=$3
  #
  # Ingreso del dato
  i_ingcor=0
  while [ $i_ingcor -eq 0 ]
  do
	  clear
	  echo "\n"
	  echo "$S Usuario: $user"
	  echo "$S -----------------------------------------"
	  echo "$S     $m_titulo"
	  echo "$S -----------------------------------------"
	  echo "$S $m_texto: \c"
	  read m_variable
	  echo
          if [[ -z $m_variable || $m_variable = " " ]] ; then
	     if [[ $m_opciones = "Fecha" ]] ; then
	        m_variable=$dia
	     fi
	     i_ingcor=1
          else
	     if [[ $m_opciones = "Fecha" ]] ; then
	        test $m_variable -gt 0 2>/dev/null
	        if [[ $? = 0 ]] ; then
	             i_ingcor=1
                else
		     mensajes.sh $m_menserr 1
	        fi
	     else
	        if [[ $(echo $m_opciones | grep -w $m_variable | wc -l) -ne 0 ]]
	        then
	             i_ingcor=1
	        else
		     mensajes.sh $m_menserr 1
	        fi
	     fi
	  fi
  done
