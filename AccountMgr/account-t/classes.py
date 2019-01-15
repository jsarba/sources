from simplexml import * 
import dialog

class ExecArgs:
	val = {}
	def __init__(self):
		self.val['filename'] = ""
		self.val['server'] = "localhost"
		self.val['user'] = "peb8"
		self.val['getpass'] = 0
		self.val['logfile'] = ""
	
	def set(self, TYPE, value):
		if self.val.has_key(TYPE):
			self.val[TYPE] = value

class initXml:
	def __init__(self, file):
		self.prop = ('pgrp', 'groups', 'home', 'shell', 'admin', 'roles')
		self.n_list = {}
		self.d_val = ""
		self.p_item = ""
		try:
			self.fd = xmldoc(file)
		except:
			print 'Could not open file %s' % file
			return 0

	def str_format(self, path, key, item):
		ret = path + "(clave=%s)/%s" % (key, item)
		return ret

	def get_elements(self, search):
		ret = self.fd.elements(search)
		return ret[0]

	def __del__(self):
		del self.fd

class ParseXml(initXml):
	def getall_values(self, path, parent, item):
		for element in self.fd.elements(path):
			self.__clave = element.attr(parent)
			search = self.str_format(path, self.__clave, item)
			self.__parent = self.get_elements(search)
			self.n_list[self.__clave] = "%s" % self.__parent
			
	def getkey_value(self, key, path, parent, item):
		for element in self.fd.elements(path):
			self.__clave = element.attr(parent)
			if key == self.__clave:
				try:
					search = self.str_format(path, self.__clave, item)
					self.d_val = self.get_elements(search)
				except:
					break

	def getprop_item(self, key, path, parent):
		for element in self.fd.elements(path):
			self.__clave = element.attr(parent)
			if self.__clave == key:
				separator = ''
				for j in self.prop:
					search = self.str_format(path, self.__clave, j)
					try:
						self.__parent = self.get_elements(search)
						self.p_item += separator
						self.p_item += "%s=\'%s\'" % (j, self.__parent)
					except:
						pass
					separator = ' '


class Object:
	name = {}
	def __init__(self):
		self.value = {}
		self.name['X'] = "Group"
		self.name['Y'] = "Username"
		self.name['Z'] = "Profile"
		self.name['W'] = "Password"
		self.name['N'] = "Full name"

class TaskSave(Object):
	def __init__(self, name):
		self.obj = Object()
		self.cmdlist = []
		self.tsk_name = name
	
	def add_cmdlist(self, key, cmd):
		self.cmdlist.append([key, cmd])
	
	def add_objvalue(self, obj, val):
		self.obj.value[obj] = val
	
	def del_objvalue(self, obj):
		del self.obj.value[obj]

	def __del__(self):
		self.tsk_name = ""
		del self.cmdlist
		del self.obj

class initScreen:
	def __init__(self):
		self.h = 0
		self.d = dialog.Dialog(dialog="dialog", compat="dialog")

	def makemenu(self, st, t, h, mh, w, cs):
		return self.d.menu(st, height=h, menu_height=mh, width=w, choices=(cs), title=t)

	def checklist(self, st, t, h, lh, w, cs):
		return self.d.checklist(text=st, height=h, width=w, list_height=lh, choices=(cs), title=t)

	def fileselect(self, t, h, w, d):
		return self.d.fselect(d, h, w, title=t)

class InstanceWM(initScreen):
	def tittle(self, name):
		self.d.add_persistent_args(["--backtitle", name])

	def SimpleMenu(self, topmenu, subhead, header,  w):
		submenu = []
		mh = len(topmenu)
		h = mh + 7
		for menu in topmenu:
			submenu.extend([[ menu[0], ""]])

		if subhead:
			subhead = "Actual task:%s" % subhead
		else:
			subhead = ""

		(self.code, self.tag ) = self.makemenu(subhead, header, h, mh, w, submenu)

	def MultiMenu(self, topmenu, subhead, header, w):
		mh = len(topmenu)
		if self.h == 0 or self.h < mh:
			self.h = mh + 9
		return self.makemenu(subhead, header, self.h, mh, w, topmenu)

	def MultiList(self, options, subhead, header, w):
		#lh = len(options) / 2
		#h = lh + 7
		return self.checklist(subhead, header, 0, 0, w, options)

	def FileSelect(self, header, dir):
		while 1:
			import os
			root_dir = os.sep
			if dir and dir[-1] != os.sep:
				dir = dir + os.sep

			(code, path) = self.fileselect(header, 10, 50, dir)
			if code in (self.d.DIALOG_CANCEL, self.d.DIALOG_ESC):
				return
			else:
				if not os.path.isfile(path):
					return 
				else:
					return path

	def key_handle(self):
		d = self.d	# shorcut
		if self.code in (d.DIALOG_CANCEL, d.DIALOG_ESC):
			return
		else:
			return self.tag
