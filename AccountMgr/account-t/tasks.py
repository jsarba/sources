import cPickle as pick
from classes import *
from profile import *
import commands
import re, sys, os

SERVERS = 'servers.xml'
serv_item = 'servers/server'
DIRECTIVES = 'directivas.xml'
dir_item = 'directives/directive'
parent = 'clave'

BIN_DIRS = './bin/'
PASS_GEN = BIN_DIRS + "/pass 8"
NSSH = BIN_DIRS + "nssh -l %s -p %s %s %s %s %s >> %s"

TASK_DIRS = './tasks'
EXEC_DIRS = './execs'

def SetAllObjects(s, dxml, tasks, tsv, MODIFY):
	if MODIFY:
		new_obj = Object()
		new_obj.value = tsv.obj.value.copy()
		tsv.obj.value.clear()

	for k in tasks:
		dxml.getkey_value(k, dir_item, parent, 'name')
		if not dxml.d_val: return 0
		directive = dxml.d_val

		dxml.getkey_value(k, dir_item, parent, 'cmd')
		if not dxml.d_val: return 0
		cmdline = "%s" % dxml.d_val

#	Solo recorremos la leyenda de la directiva en busqueda de objetos
#	El comando cambio de password no contiene el objeto %W%
		raw = re.compile('%[a-z]+%', re.IGNORECASE)
		iteret = raw.finditer("%s" % directive)

		subraw = re.compile('[a-z]+', re.IGNORECASE)
		# por cada objeto detectado
		for i in iteret:
			tent = ""
			subret = subraw.search("%s" % i.group())
			j = subret.group()

			# si el objeto ya fue seteado no lo volvemos a asignar
			if MODIFY:
				tent = new_obj.value[j]
			else:
				if tsv.obj.value.has_key(j):
					pass

			if not tsv.obj.value.has_key(j):
				# la password la mostramos con fines administrativos
				if tsv.obj.name[j] == 'Password':
					tent = commands.getoutput(PASS_GEN)
				# El profile debemos elegirlo de un listado
				# cuando se guarda la tarea el nombre del profile se pierde
				if tsv.obj.name[j] == 'Profile':
					(code, tag) = get_profile(s, tsv.tsk_name)
				else:
					(code, tag) = s.d.inputbox("%s\n\n" % directive + tsv.obj.name[j] + 
						"(%s):" % i.group(), title="Objects", width=50, init=tent)

				if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC) or not tag:
					return 0

				tsv.add_objvalue(j, tag)

		# agregamos el comando en su estado puro, es decir con los objetos
		# sin reemplazar
		if not MODIFY:
			tsv.add_cmdlist(k, cmdline)

	if MODIFY:
		del new_obj
	return 1

def task_create(s, title, tsv):
	ls_key = []

	dxml = ParseXml(DIRECTIVES)
	dxml.getall_values(dir_item, parent, 'name')
	
	n_list = len(dxml.n_list)
	added = ""
	
	# vamos agregando directivas una a una
	while n_list:
		name_list = dxml.n_list.items()
		if len(ls_key) > 0:
			name_list.append(["APPLY", "Apply changes"])

		(code, tag) = s.MultiMenu(name_list, 
			"Task Name: %s\nAdded:\n%s" % (tsv.tsk_name, added), "Choice directives", 60)

		if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC):
			return 
		else:
			if tag == 'APPLY':
				break
			else:
				n_list -= 1
				added = added + " - %s\n" % dxml.n_list[tag]
				ls_key.append(tag)
				del dxml.n_list[tag]

	# seteamos los objetos del listado de directivas
	ret = SetAllObjects(s, dxml, ls_key, tsv, 0)
	del dxml
	return ret

def task_save(s, title, tsv):
	global TASK_DIRS
	(code, tag) = s.d.inputbox("FileName:", title=title, height=8, init=tsv.tsk_name)	

	if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC) or not tag:
		return 0

	filename = "%s/%s.dat" % (TASK_DIRS, tag)
	fd = open(filename, 'w')
	pick.dump(tsv, fd)
	fd.close()
	return 1

def task_load(s, title):
	file = s.FileSelect(title, TASK_DIRS)
	# Agregar la busqueda de un listado de archivos
	try:
		fd = open(file, 'r')
	except:
		s.d.msgbox("File error", title=title, height=5)
		return

	ret = pick.load(fd)
	fd.close()
	return ret

def Exec(s, rxml, tske, arg):
	arg.val['user'] = 'peb8'
	data = 'readdata'

	equipo = arg.val['server']
	filename = arg.val['filename']

	if arg.val['getpass']:
		(code, password) = s.d.passwordbox("%s@%s's password:" % (arg.val['user'], arg.val['server']), 
			title=title, height=8)
		if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC):
			return 0
		# preguntar la password de logeo de quien autentica
		line = NSSH % (arg.val['user'], rxml.n_list[equipo], 
			equipo, filename, password, tske.obj.value['W'], arg.val['logfile'])
	else:
		line = NSSH % (arg.val['user'], rxml.n_list[equipo], 
			equipo, filename, 'none', 'none', arg.val['logfile'])

	f = os.popen(line)

	for line in f.readlines():
		pass
#		lf.write(line)

	f.close()

	return 1


def task_run(s, title, tske):
	ls_srv = []
	rxml = ParseXml(SERVERS)
	arg = ExecArgs()
	
	rxml.getall_values(serv_item, parent, 'port')
	r_list = len(rxml.n_list)

	server_list = rxml.n_list.keys()

	for k in server_list:
		ls_srv.append((k, "%s" % rxml.n_list[k], 0))
		
	(code, tag ) = s.MultiList(ls_srv, 
		"Task Name: %s" % tske.tsk_name, "Choice Servers", 45)
	if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC) or not tag:
		return 

	raw = re.compile('%[a-z]+%', re.IGNORECASE)

	(code, ticket) = s.d.inputbox("Remedy ticket id:", title="Log file", height=8, width=50)
	if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC) or not ticket:
		return 

	logfile = "%s/%s.logs" % (EXEC_DIRS, ticket)
	arg.val['logfile'] = logfile
	open(logfile, 'w').close()

	pid = os.fork()
	if pid == 0:
		# los equipos elegidos
		for arg.val['server'] in tag:

			arg.val['filename'] = "%s/%s.%s" % (EXEC_DIRS, tske.tsk_name, arg.val['server'])
			fd = open(arg.val['filename'], 'w')
		
			# Armamos el archivo temporal para el argumento de nssh
			for de in tske.cmdlist:
				directive = "%s" % de[1]
				iteret = raw.finditer("%s" % directive)
				for i in iteret:
					subraw = re.compile('[a-z]+', re.IGNORECASE)
					subret = subraw.search("%s" % i.group())
					k = subret.group()	
					pure = re.compile(i.group())
					cmdline = "%s" % (pure.sub(tske.obj.value[k], directive))
					# comentario
					fd.write('< ' + cmdline + '\n')
					# linea ejecutada
					fd.write(cmdline + '\n')

				if de[0] == 'apwuy':
					arg.val['getpas'] = 1	

			fd.close()
			
			if not Exec(s, rxml, tske, arg):
				break

	s.d.tailbox(logfile, height=20, width=0, title='Running task:%s' % tske.tsk_name)

	del rxml
	del arg

def task_modify(s, title, tske):
	ls_key = []	
	dxml = ParseXml(DIRECTIVES)

	for k in tske.cmdlist:
		ls_key.append(k[0])

	ret = SetAllObjects(s, dxml, ls_key, tske, 1)

	del ls_key
	del dxml
	return ret

