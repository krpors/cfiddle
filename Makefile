all:
	$(MAKE) -C ./daemon/ all
	$(MAKE) -C ./fire/ all
	$(MAKE) -C ./hashtable/ all
	$(MAKE) -C ./kbd/ all
	$(MAKE) -C ./list/ all
	$(MAKE) -C ./mcast/ all

clean:
	$(MAKE) -C ./daemon/ clean
	$(MAKE) -C ./fire/ clean
	$(MAKE) -C ./hashtable/ clean
	$(MAKE) -C ./kbd/ clean
	$(MAKE) -C ./list/ clean
	$(MAKE) -C ./mcast/ clean
