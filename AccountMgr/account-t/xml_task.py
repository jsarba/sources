from simplexml import *

MENU_FILE = 'menu.xml'
main_item = 'ppal/menu'
sub_item = 'submenu'

def xmlopen(filename):
	try:
		return xmldoc(filename)
	except:
		print 'Could not open file %s' % filename
		sys.exit(2)


def menu_array(d, path, subpath):
	topbar = []
	sublist = []

	for element in d.elements(path):
		topbar.extend([[ element.attr('name'), element.attr('func') ]])
		sub_menu = main_item + "(name=" + element.attr('name') + ")/" + subpath
		subbar = [] 
		for subel in d.elements(sub_menu):
			subbar += [[subel.attr('name'),subel.attr('func')]]
		if subbar:
			sublist += [subbar]

	return (topbar, sublist)
