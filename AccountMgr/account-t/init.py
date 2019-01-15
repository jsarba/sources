import sys, os, string
import re
from classes import *
from xml_task import *
from tasks import *

VERSION = '0.1r'
BANNER = 'Account Manager Version ' 

def Exit():
    sys.exit(0)

# determinamos el index de la opcion
def getoption(ret, topbar):
	count = 0
	for i in topbar:
		if ret in (i):
			return count
		count += 1
	return 0

def dispatch(s, func, option):
	global task
	global tske
	rep = re.compile('\(\)')
	
	if 'Tasks/New' == option:
		if not task:
			(code, tsk_name) = s.d.inputbox("Name", title=option, height=8)
			if not tsk_name:
				return

			tske = TaskSave(tsk_name)
			args = '(s, option, tske)'
			func_exec = "%s" % rep.sub(args, func, 1)
			if eval(func_exec):
				return tske.tsk_name
			else:
				del tske
				return
		else:
			s.d.msgbox("Already a task exists.\nYou must save this task first", 
				width=50, height=6, title=option)
			return tske.tsk_name

	elif 'Tasks/Save' == option:
		if task:
			args = '(s, option, tske)'
			func_exec = "%s" % rep.sub(args, func, 1)
			if eval(func_exec):
				del tske
				return
			else:
				return tske.tsk_name
		else:
			s.d.msgbox("You must create a task first", 
				width=50, height=5, title=option)

	elif 'Tasks/Load' == option:
		if not task:
			args = '(s, option)'
			func_exec = "%s" % rep.sub(args, func, 1)
			objs = Object()
			tske = eval(func_exec)
			if tske:
				tske.obj.name = objs.name
				return tske.tsk_name
			return
		else:
			s.d.msgbox("Already a task exists.\nYou must save this task first", 
				width=50, height=6, title=option)
			return tske.tsk_name

	elif 'Tasks/Modify' == option:
		if not task:
			s.d.msgbox("You must create or load a task first", 
				width=50, height=5, title=option)
		else:
			args = '(s, option, tske)'
			func_exec = "%s" % rep.sub(args, func, 1)
			if eval(func_exec):
				return tske.tsk_name
			else:
				return

	elif 'Tasks/Run' == option:
		if not task:
			s.d.msgbox("You must create or load a task first", 
				width=50, height=5, title=option)
			return
		else:
			args = '(s, option, tske)'
			func_exec = "%s" % rep.sub(args, func, 1)
			eval(func_exec)
			return tske.tsk_name


def choiceOpt(s, topbar, sublist, w, tittle):
	global task
	while 1:
		s.SimpleMenu(topbar, task, tittle, w)
		index = 0
		ret = s.key_handle()
		if not ret:
			return 0;
		else:
			index = getoption(ret, topbar)
			func_name = topbar[index][1]
			if ret == "Exit":
				eval(func_name)
			if "(" in func_name:
#				try:
				task = dispatch(s, func_name, "%s/%s" % (tittle, ret))
#				except:
#					pass
			else:
				choiceOpt(s, sublist[index], task, 40, ret)
	return 1

if __name__ == '__main__':
	task = ""
	d = xmlopen(MENU_FILE)
	(topbar ,sublist) = menu_array(d, main_item, sub_item)
	del d
	
	s = InstanceWM()
	s.tittle(BANNER)

	while choiceOpt(s, topbar, sublist, 40, "Main"):
		pass

	del s
	Exit()
