#!/usr/bin/env python
# -*- coding: utf-8 -*-
#daemon reference 
#http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/66012

#Copyright Shellcode SRL
#This file is to be released using the GPLv2 licence #see COPYING

# Desarrollo - Ignacion Marambio Catan - ignaciom@shellcode.com.ar
# Revision 25/01/2008 - Jonathan SArba - sarbaj@shellcode.com.ar 
# Desarrollo 28/03/2008 - Correccion - Jonathan Sarba - sarbaj@shellcode.com.ar

import sys
import os
import SimpleXMLRPCServer
import syslog
import base64
import getpass
import os
import socket
import sys
import traceback
from binascii import hexlify

import paramiko

PATH_CONFIG='/opt/secure-transfer/conf/'	
XML_PORT=8888

'''This module is used to fork the current process into a daemon.
    Almost none of this is necessary (or advisable) if your daemon 
    is being started by inetd. In that case, stdin, stdout and stderr are 
    all set up for you to refer to the network connection, and the fork()s 
    and session manipulation should not be done (to avoid confusing inetd). 
    Only the chdir() and umask() steps remain as useful.
    References:
        UNIX Programming FAQ
            1.7 How do I get my program to act like a daemon?
                http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC16
    
        Advanced Programming in the Unix Environment
            W. Richard Stevens, 1992, Addison-Wesley, ISBN 0-201-56317-7.
    '''

def daemonize (stdin='/dev/null', stdout='/dev/null', stderr='/dev/null'):
	'''This forks the current process into a daemon.
	The stdin, stdout, and stderr arguments are file names that
	will be opened and be used to replace the standard file descriptors
	in sys.stdin, sys.stdout, and sys.stderr.
	These arguments are optional and default to /dev/null.
	Note that stderr is opened unbuffered, so
	if it shares a file with stdout then interleaved output
	may not appear in the order that you expect.
	'''
	# Do first fork.
	try: 
		pid = os.fork() 
		if pid > 0:
			sys.exit(0) # Exit first parent.
	except OSError, e: 
		sys.stderr.write ("fork #1 failed: (%d) %s\n" % (e.errno, e.strerror)    )
		sys.exit(1)
        
    # Decouple from parent environment.
	os.chdir("/") 
	os.umask(0) 
	os.setsid() 
    
    # Do second fork.
	try: 
		pid = os.fork() 
		if pid > 0:
			sys.exit(0) # Exit second parent.
	except OSError, e: 
		sys.stderr.write ("fork #2 failed: (%d) %s\n" % (e.errno, e.strerror)    )
		sys.exit(1)
        
    # Now I am a daemon!
    
    # Redirect standard file descriptors.
	si = file(stdin, 'r')
	so = file(stdout, 'a+')
	se = file(stderr, 'a+', 0)
	sys.stdout.flush()
	sys.stderr.flush()
	os.dup2(si.fileno(), sys.stdin.fileno())
	os.dup2(so.fileno(), sys.stdout.fileno())
	os.dup2(se.fileno(), sys.stderr.fileno())

class xmlrpc_hf:
	def bring(self, rpath, lpath):
		conn = sftp_part()
		try:
			conn.connect()
#			traceback.print_exc()
			conn.bringfile(rpath,lpath)
			conn.disconnect()
		except IOError, e:
			syslog.syslog(syslog.LOG_ERR,"input/output error: "+e.strerror)
			return e.strerror
		except Exception, e:
			traceback.print_exc()
			syslog.syslog(syslog.LOG_ERR,"problems connecting to the ssh server\n")
			return "problems connecting to the server"

		syslog.syslog(syslog.LOG_NOTICE,"the file "+lpath+" has been bringed from "+rpath+" successfully\n")
		return "done"

	def pushanddel(self, lpath, rpath, action=0):
		conn = sftp_part()
		syslog.syslog(syslog.LOG_ERR, "pushanddel:"+ lpath + " a " + rpath )
		try:
			conn.connect()
			conn.putfile(lpath, rpath, action)
			conn.disconnect()
#		except IOError, e:
#			syslog.syslog(syslog.LOG_ERR,"input/output error: "+e.strerror)
#			return e.strerror
		except Exception, e:
			traceback.print_exc()
			syslog.syslog(syslog.LOG_ERR,"problems connecting to the ssh server\n")
			return "problems connecting to the server" 
		syslog.syslog(syslog.LOG_NOTICE,"the file "+lpath+" has been copied to "+rpath+" via pushanddel (action="+str(action)+") successfully\n")
		return "done"
		
	

class sftp_part:
	def __init__(self, cfile='%s/x-sgw.conf' % PATH_CONFIG):
		self.t=None
		self.sftp=None
		self.rdir=[]
		try:
			d={}
			try:
				f=open(cfile)
				l=f.readlines()
			except:
				syslog.syslog(syslog.LOG_EMERG,"config file ( "+cfile+" ) does not exist\n")
				exit(1)
				
			for i in l:
				i=i.strip()
				sst=i.split(':',1)
				d[sst[0]]=sst[1]

			self.username=d['username']
			self.hostname=d['hostname']
			self.hostsfile=d['hostsfile']
			self.port=int(d['port'])
			self.privkeypath=d['privkeypath']
			self.ldir=d['ldir']
			# Con la accion 0 se copia primero a auditoria y no se borra el archivo origen
			# con la accion 1 se copia al repositorio y se borra el archivo origen
			self.rdir=[d['rdir_audit'], d['rdir']]

		except:
			syslog.syslog(syslog.LOG_EMERG,"config file error\n")
		#	raise
			exit(1)
		
	def connect(self):
		privkey = paramiko.RSAKey.from_private_key_file (self.privkeypath)
		host_keys = paramiko.util.load_host_keys (self.hostsfile)
  		if host_keys.has_key (self.hostname):
    			hostkeytype = host_keys[self.hostname].keys()[0]
    			hostkey = host_keys[self.hostname][hostkeytype]
		self.t = paramiko.Transport ((self.hostname, self.port))
		self.t.connect (hostkey=hostkey, pkey=privkey, username=self.username)
		self.sftp = paramiko.SFTPClient.from_transport (self.t)

	def bringfile(self,rpath,lpath):
		syslog.syslog(syslog.LOG_INFO,"the file "+rpath+" will be bringed from "+self.rdir[1]+"\n")
		remotefile=self.rdir[1] + "/" + rpath.replace("../", "")
		data = self.sftp.open(remotefile).read()
		localpath=(self.ldir+lpath).replace("../","")
		open(localpath, 'w').write(data)

	def putfile(self,lpath, rpath, act):
		syslog.syslog(syslog.LOG_INFO,"the file "+(self.ldir+lpath)+" will be copied to "+rpath+" (action="+str(act)+")\n")
		print "local file :" + (self.ldir+lpath)
		print "remote file:" + (self.rdir[int(act)]+rpath)
		data=open(self.ldir+lpath,"r").read()
		a=rpath.replace("../","").split('/')
		destination_path="/".join(a[:-1])
		rpath=destination_path+"/"+a[-1]
		mkdir_file=self.rdir[int(act)]+"/"+destination_path
		# si el directorio existe en el destino dara un IOError
		try:
			self.sftp.mkdir(mkdir_file)
		except IOError:
			print "Directory already exist:" + mkdir_file
		print "remote mkdir :" + mkdir_file
		self.sftp.open((self.rdir[int(act)] + "/" + rpath),"w").write(data)
		if act==1:
			os.unlink((self.ldir + lpath).replace("../",""))		

	def disconnect(self):
		self.t.close()

def main ():
	xo= xmlrpc_hf()

	try:
	   server = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", XML_PORT))
	   server.register_instance(xo)
	except:
	   syslog.syslog(syslog.LOG_EMERG,"binding port error")
	   return 

	try:
		server.serve_forever()
	except KeyboardInterrupt:
		return
		
	#sys.stdout.write ('' )
    
if __name__ == "__main__":
	daemonize(stdin='/dev/null', stdout='/dev/null', stderr='/dev/null')
	syslog.openlog('siti-gw', 0, syslog.LOG_DAEMON)
	main()
	syslog.closelog()
	

