from classes import *

PROFILES = 'perfiles.xml'
pro_item = 'perfiles/perfil'
pro_parent = 'clave'

def get_profile(s, task_name):
	pxml = ParseXml(PROFILES)
	pxml.getall_values(pro_item, pro_parent, 'name')

	pro_list = pxml.n_list.items()

	(code, tag) = s.MultiMenu(pro_list,
		"Task: %s" % task_name, "Choice profile", 60)

	if code in (s.d.DIALOG_CANCEL, s.d.DIALOG_ESC):
		return
	pxml.getprop_item(tag, pro_item, pro_parent)

	if pxml.p_item:
		tag = pxml.p_item

	return (code, tag)
