/*
    SyslogDog. Concentrator of systems logs
    Copyright (C) 2005  Jonathan Sarba.

    This file is part of SyslogDog.
    SyslogDog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SyslogDog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SyslogDog; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "dbase.h"

int checkOCIerror(char *szDoing, OCIError *ociError, sword nStatus)
{
  text txtErrorBuffer[512] = { 0 };
#ifdef _ORACLE10
  sb4 nErrorCode;
#else
  ub4 nErrorCode;
#endif

  // no logueamos los errores de este tipo a la base.
  // el OCILogon quedaria en un loop
  switch (nStatus)
  {
    case OCI_SUCCESS:
      break;
    case OCI_SUCCESS_WITH_INFO:
      debugf(DEBUG_L1, -1, "(%s) Error - OCI_SUCCESS_WITH_INFO\n", szDoing);
      break;
    case OCI_NEED_DATA:
      debugf(DEBUG_L1, -1, "(%s) Error - OCI_NEED_DATA\n", szDoing);
      break;
    case OCI_NO_DATA:
      debugf(DEBUG_L1, -1, "(%s) Error - OCI_NODATA\n", szDoing);
      break;
    case OCI_ERROR:
      OCIErrorGet(ociError, (ub4)1, (text *)NULL, &nErrorCode, txtErrorBuffer, (ub4)sizeof(txtErrorBuffer)-1, OCI_HTYPE_ERROR);
      debugf(DEBUG_L1, -1, "(%s) Error - %s", szDoing, txtErrorBuffer);
      break;
    case OCI_INVALID_HANDLE:
      debugf(DEBUG_L1, -1, "(%s) Error - OCI_INVALID_HANDLE\n", szDoing);
      break;
    case OCI_STILL_EXECUTING:
      debugf(DEBUG_L1, -1, "(%s) Error - OCI_STILL_EXECUTE\n", szDoing);
      break;
    default:
      break;
  }
  return nStatus;
}

// escape common character (injection evasive)
int oracle_escape_string(char *dest, char *src, int src_lenght)
{
    int ret = 0;

	// TODO_WARN: Ningun mensaje contendra un retorno de linea??
    while ( *src && (int )*src != '\n' && (int )*src != '\r' )
    {
        if ( strchr(ESCAPED_STR, *src) != NULL )
			*dest++ = '\"';	
		else
            *dest++ = *src;

       	*src++;
		ret++;
    }
    *dest = '\0';

    return ret;
}

#ifdef _ORACLE10
bool oracle_init(ORACLE *con_ptr)
{
	(void)OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
		(dvoid * (*)(dvoid *, size_t))0,
		(dvoid * (*)(dvoid *, dvoid *, size_t))0,
		(void (*)(dvoid *, dvoid *))0 );
	
	(void)OCIEnvInit((OCIEnv **)con_ptr->ociEnv, OCI_DEFAULT, (size_t)0, (dvoid **)0);
	(void)OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociErr, OCI_HTYPE_ERROR, (size_t)0, (dvoid **)0);
	(void)OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociSrv, OCI_HTYPE_SERVER, (size_t)0, (dvoid **)0);
	(void)OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociSrv, OCI_HTYPE_SVCCTX, (size_t)0, (dvoid **)0);

	// TODO: testing1; how could i check the handles error?
	return TRUE;

}
#else
bool oracle_init(ORACLE *con_ptr)
{
	/* init */
	if ( OCIInitialize((ub4)OCI_DEFAULT, (dvoid *)0, (dvoid *)0, (dvoid *)0, (dvoid *)0) ) 
		return FALSE;

    /* env */
    if ( OCIEnvInit(&con_ptr->ociEnv, OCI_DEFAULT, 0, (dvoid **)0) )
        return FALSE;

	/* alloc */	
    if ( OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociErr, OCI_HTYPE_ERROR, (size_t)0, (dvoid **)NULL) )
        return FALSE;
    if ( OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociSrv, OCI_HTYPE_SVCCTX, (size_t)0, (dvoid**)NULL) )
        return FALSE;

	return TRUE;
}
#endif

// logica inversa, heredara de mysql_ping
bool oracle_ping(ORACLE *con_ptr)
{
#ifdef _ORACLE10
	if ( checkOCIerror("Prepare ping", con_ptr->ociErr, 
		OCIStmtPrepare(con_ptr->ociStat, con_ptr->ociErr, 
			(OraText *)"select 'dbuped' from dual", (ub4)25, OCI_NTV_SYNTAX, OCI_DEFAULT)) != 0 )
			return TRUE;	
#else
	if ( checkOCIerror("Prepare ping", con_ptr->ociErr, OCIStmtPrepare(con_ptr->ociStat, con_ptr->ociErr, 
		"select 'dbuped' from dual", (ub4)25, OCI_NTV_SYNTAX, OCI_DEFAULT)) != 0 )
			return TRUE;	
#endif

	if ( checkOCIerror("Execute ping", con_ptr->ociErr, OCIStmtExecute(con_ptr->ociSrv, con_ptr->ociStat, con_ptr->ociErr, 
		(ub4)0, (ub4)0, (CONST OCISnapshot *)NULL, (OCISnapshot *)NULL, OCI_DESCRIBE_ONLY)) != 0 )
			return TRUE;

	return FALSE;
}


void oracle_close(ORACLE *con_ptr)
{
	OCILogoff(con_ptr->ociSrv, con_ptr->ociErr);

	// TODO_WARN: no deberia de hacerse un free si nunca fue inicializado.
	// Depurar en flujo..
	OCIHandleFree((dvoid *)con_ptr->ociStat, OCI_HTYPE_STMT);
	OCIHandleFree((dvoid *)con_ptr->ociSrv, OCI_HTYPE_SVCCTX);
	OCIHandleFree((dvoid *)con_ptr->ociErr, OCI_HTYPE_ERROR);
	OCIHandleFree((dvoid *)con_ptr->ociEnv, OCI_HTYPE_ENV);
}

bool Oracle_Connect(ORACLE *con_ptr)
{
#ifdef _ORACLE10
    /* logon */
    if ( checkOCIerror("Logon", con_ptr->ociErr, 
		OCILogon(con_ptr->ociEnv, con_ptr->ociErr, &con_ptr->ociSrv,
			(OraText *)dbi.db_user, (ub4)strlen(dbi.db_user),                        
			(OraText *)dbi.db_pass, (ub4)strlen(dbi.db_pass),
			(OraText *)dbi.db_server, (ub4)strlen(dbi.db_server))) != 0 )
		return FALSE;
#else
    if ( checkOCIerror("Logon", con_ptr->ociErr, OCILogon(con_ptr->ociEnv, con_ptr->ociErr, &con_ptr->ociSrv, 
		dbi.db_user, strlen(dbi.db_user), dbi.db_pass, strlen(dbi.db_pass), dbi.db_server, strlen(dbi.db_server))) != 0 )
		return FALSE;
#endif

	/* Statement */
	if ( OCIHandleAlloc((dvoid *)con_ptr->ociEnv, (dvoid **)&con_ptr->ociStat, OCI_HTYPE_STMT, (size_t)0, (dvoid **)0) != 0 )
		return FALSE;

	return TRUE;
}


bool Oracle_Write(DataToDb *msg, ORACLE *inst)
{
    char query[MAX_QUERY] = { 0 };
	char escape[MAX_SYSLOG_LINE] = { 0 };
	char ts_tmp[TS_LEN+1] = { 0 };

	oracle_escape_string(escape, msg->msgline, strlen(msg->msgline));
	/* La funcion to_date definida por oracle requiere un string
	 * from oracle document....
	 * to_date (string1, [format_mask], [nls_language] )
	*/
	snprintf(ts_tmp, TS_LEN, "%lld", msg->timestamp);

    snprintf(query, sizeof(query),
	 "INSERT INTO %s (dia,facility,priority,hostname,message) VALUES(TO_DATE('%s', 'YYYYMMDDHH24MI_SS'), '%d', '%d', '%ld', '%s')", 
	 dbi.db_table, ts_tmp, msg->clase.fac, msg->clase.pri, msg->ipaddr , escape);

#ifdef _ORACLE10
	if ( checkOCIerror("Write Prepare", inst->ociErr, 
	 	OCIStmtPrepare(inst->ociStat, inst->ociErr, (OraText *)query, (ub4)strlen(query), OCI_NTV_SYNTAX, OCI_DEFAULT)) != 0 ) 
		return FALSE;
#else
	if ( checkOCIerror("Write Prepare", inst->ociErr, 
	 OCIStmtPrepare(inst->ociStat, inst->ociErr, query, (ub4)strlen(query), OCI_NTV_SYNTAX, OCI_DEFAULT)) != 0 ) 
		return FALSE;
#endif

	if ( checkOCIerror("Write Execute", inst->ociErr, 
	 OCIStmtExecute(inst->ociSrv, inst->ociStat, inst->ociErr, (ub4)1, (ub4)0, 
	 (CONST OCISnapshot *)NULL, (OCISnapshot *)NULL, OCI_COMMIT_ON_SUCCESS)) != 0 )
		return FALSE;

	return TRUE;
}


bool OracleConn(ORACLE *conn, int action)
{
	switch(action)
	{
		case OPEN:
			if (oracle_init(conn) && Oracle_Connect(conn)) return TRUE;
			break;
		case CLOSE:
			if ( oracle_ping(conn) == 0 ) oracle_close(conn);
			return TRUE;
			break;
		case RECONNECT:
			(void )OracleConn(conn, CLOSE);
			return OracleConn(conn, OPEN);
			break;
	}
	return FALSE;
} 
