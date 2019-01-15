SUBDIRS = src etc

DATA_DIR = $(prefix)/var/spool/$(PACKAGE)
LOG_DIR = $(prefix)/var/log/

INSTALL_PROGRAM = $(INSTALL) -m

sldog_init = /etc/init.d/sldog
sldog_script = $(prefix)/etc/sldog.sh
sldog_up_rc = S10sldog
sldog_down_rc = K80sldog

install: all
	$(INSTALL_PROGRAM) 750 -s $(srcdir)/src/sldog $(prefix)/sbin/

	-@if test '!' -f $(prefix)/etc/cip-sldog.conf ; then \
		$(INSTALL_PROGRAM) 600 $(srcdir)/etc/cip-sldog.conf $(prefix)/etc/ ; \
	fi

	-@if test '!' -f $(prefix)/etc/db-sldog.conf ; then \
		$(INSTALL_PROGRAM) 600 $(srcdir)/etc/db-sldog.conf $(prefix)/etc/ ; \
	fi
	
	-@if test '!' -f $(sldog_script) ; then \
		$(INSTALL_PROGRAM) 755 $(srcdir)/etc/sldog.sh.ori $(sldog_script) ; \
		perl -pi -e "s,PREFIX,$(prefix)/sbin," $(sldog_script) ; \
	fi

	-@if test '!' -z "$(ORACLE_HOME)" ; then \
		perl -pi -e "s,#ORACLE_HOME,export\ ORACLE_HOME=$(ORACLE_HOME)," $(sldog_script) ; \
		perl -pi -e "s,#ORACLE_BASE,export\ ORACLE_BASE=$(ORACLE_BASE)," $(sldog_script) ; \
	fi
	
	-@if test -f /etc/rc0.d/$(sldog_down_rc) ; then rm -r /etc/rc0.d/$(sldog_down_rc) 2>/dev/null ; fi
	-@if test -f /etc/rc2.d/$(sldog_up_rc) ; then rm -r /etc/rc2.d/$(sldog_up_rc) 2>/dev/null ; fi
	-@if test -f /etc/rc3.d/$(sldog_up_rc) ; then rm -r /etc/rc3.d/$(sldog_up_rc) 2>/dev/null ; fi
	-@if test -f /etc/rc6.d/$(sldog_down_rc) ; then rm -r /etc/rc6.d/$(sldog_down_rc) 2>/dev/null ; fi
	-@if test -f $(sldog_init) ; then rm -r $(sldog_init) 2>/dev/null ; fi

	-cp $(sldog_script) $(sldog_init) 
	-ln -s $(sldog_init) /etc/rc0.d/$(sldog_down_rc) 
	-ln -s $(sldog_init) /etc/rc2.d/$(sldog_up_rc) 
	-ln -s $(sldog_init) /etc/rc3.d/$(sldog_up_rc) 
	-ln -s $(sldog_init) /etc/rc6.d/$(sldog_down_rc)
	
	-@if test '!' -d $(DATA_DIR) ; then \
		mkdir -p $(DATA_DIR) ;\
	fi

	-@if test '!' -d $(LOG_DIR) ; then \
		mkdir -p $(LOG_DIR) ;\
	fi
