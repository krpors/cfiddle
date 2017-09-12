all:
	$(MAKE) -C ./mcast/ all
	$(MAKE) -C ./hashtable/ all

clean:
	$(MAKE) -C ./mcast/ clean
	$(MAKE) -C ./hashtable clean
