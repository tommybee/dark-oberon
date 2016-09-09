build:
	cd src && make

clean:
	cd src && make clean
	rm -f doberon

DESTDIR = dark-oberon-1.0.2-RC1

prepare-release:
	rm -rf $(DESTDIR)
	mkdir $(DESTDIR)
	cp Makefile README $(DESTDIR)
	cp -r dat libs maps races schemes src $(DESTDIR)
	find $(DESTDIR) -name CVS | while read dir ; do rm -rf $$dir ; done
	find $(DESTDIR) -name .cvsignore -exec rm -f {} \;
	find $(DESTDIR) -name .checking -exec rm -f {} \;
	find $(DESTDIR) -name .#* -exec rm -f {} \;
	rm -f $(DESTDIR)/src/devel_stats.html
	cd $(DESTDIR) && make clean
	cd $(DESTDIR)/src && sh create_makefile.sh
	mkdir $(DESTDIR)/docs
	cp docs/documentation/Final/*.pdf $(DESTDIR)/docs
	cp docs/documentation/Quick_Guide_EN.txt $(DESTDIR)/docs
	tar -czvf $(DESTDIR).tar.gz $(DESTDIR)
