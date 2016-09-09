SOUND ?= 1
DEBUG ?= 0
WINDOWS ?= 1
UNIX ?= 0

CXX=i686-w64-mingw32-g++
#CXX=g++

CPP = $(CXX) $(CPPFLAGS) $(INCLUDES) $(DEFINES)
CPPFLAGS = -g -Wall -O
#DEFINES = -DDATA_DIR='""' -DUNIX=$(UNIX) -DSOUND=$(SOUND) -DDEBUG=$(DEBUG) -DWINDOWS=$(WINDOWS) 
#DEFINES = -DDATA_DIR='""' -DSOUND=$(SOUND) -DDEBUG=$(DEBUG) -DNEW_GLFW3=1
#INCLUDES = -I/usr/X11R6/include -I/usr/X11R6/include/GL -I../libs 
#LIBPATHS = -L/usr/X11R6/lib -L/usr/lib -L/usr/local/lib -L../libs
#LIBRARIES = -pthread -lglfw -lGL -lX11 -lGLU 
#INCLUDES = -I/mingw32/include -I/mingw32/include/GLFW -I../libs/fmod3/include -I../libs/tinycthread/include
#LIBPATHS = -L/mingw32/lib -L../libs/fmod3/lib -L../libs/tinycthread/lib
DEFINES = -DDATA_DIR='""' -DSOUND=$(SOUND) -DDEBUG=$(DEBUG)
INCLUDES = -I/mingw32/include -I/mingw32/include/GLFW -I../libs/fmod3/include -I../libs/glfw-legacy/include/GL
LIBPATHS = -L/mingw32/lib -L../libs/fmod3/lib -L../libs/glfw-legacy/lib

LIBRARIES = -static -mwindows -lmingw32 -lSDLmain -lSDL -lSDL_image -lglfw -lopengl32 -lglu32 -lfmod -s -lSDL_gfx -lSDL_mixer  -lvorbisfile -lvorbis -lmingw32 -lbz2 -lharfbuzz -lglib-2.0 -lintl -liconv -ltiff -ljpeg -llzma -lpng16 -lstdc++ -lwebp -lwinpthread -lz -larchive -lwinmm -lgdi32 -ldxguid -lasprintf -lcharset -lcrypto -lcurl -lexpat -lffi -lFLAC++ -lFLAC -lfontconfig -lformw -lfreeglut_static -lgdbm -lgettextlib -lgettextpo -lgif -lgio-2.0 -lglew32 -lglew32mx -lgmodule-2.0 -lgmp -lgmpxx -lgnurx -lgnutls -lgnutlsxx -lgobject-2.0 -lgthread-2.0 -lhistory -lhogweed -lidn -lisl -ljansson  -ljsoncpp -llua  -llzo2  -lmenuw -lmetalink -lminizip -lmpc -lmpfr -lncurses++w -lncursesw -lnettle -lnghttp2 -logg -lopenal -lpanelw -lphysfs -lpixman-1 -lreadline -lregex -lrtmp -lssh2 -lssl -lsystre -ltasn1 -ltclstub86 -ltermcap -ltheora -ltheoradec -ltheoraenc -ltkstub86 -ltre -lturbojpeg -lvorbisenc -lwebpdecoder -lwebpdemux -lwebpmux -lole32 -lws2_32
OBJECTS = doalloc.o doberon.o dobuildings.o doconfig.o dodata.o dodraw.o doengine.o doevents.o dofactories.o dofight.o dofile.o dofollower.o doforces.o dohost.o doipc.o dolayout.o doleader.o dologs.o domap.o domapunits.o domouse.o donet.o doplayers.o doraces.o doschemes.o doselection.o dosimpletypes.o dosound.o dosources.o dounits.o dowalk.o doworkers.o glfont.o glgui.o tga.o utils.o
TARGETS = ../dark-oberon

#all: tags ../dark-oberon checking
all: ../dark-oberon

checking: .checking
.checking: *.h *.cpp
	@echo Checking format of source files...
	@if egrep -l "`printf '\t'`|`printf '\r'`" *.cpp *.h; then echo "Error: Previous listed files contain illegal characters (tabs or ^M)"; false; fi
	@echo OK
	@touch .checking

clean: 
	rm -f $(OBJECTS) $(TARGETS) *core core.* tags

tags: *.h *.cpp
	-exctags * 2> /dev/null || ctags * 2> /dev/null

docs:
	rm -rf ../docs/documentation/html_local
	doxygen .doxygen.conf 2>&1 | tee .doxygen.log

../dark-oberon: $(OBJECTS)
	$(CPP) $(OBJECTS) $(LIBPATHS) $(LIBRARIES) -o ../dark-oberon

doalloc.o: doalloc.cpp cfg.h doalloc.h dologs.h
	$(CPP) -c doalloc.cpp

doberon.o: doberon.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h utils.h
	$(CPP) -c doberon.cpp

dobuildings.o: dobuildings.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dobuildings.cpp

doconfig.o: doconfig.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doconfig.cpp

dodata.o: dodata.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h tga.h
	$(CPP) -c dodata.cpp

dodraw.o: dodraw.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dodraw.cpp

doengine.o: doengine.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dofollower.h dohost.h doipc.h dolayout.h doleader.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doengine.cpp

doevents.o: doevents.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doevents.cpp

dofactories.o: dofactories.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dofactories.cpp

dofight.o: dofight.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dofight.cpp

dofile.o: dofile.cpp cfg.h doalloc.h dodata.h dofile.h doipc.h dologs.h dosimpletypes.h dosound.h glfont.h glgui.h
	$(CPP) -c dofile.cpp

dofollower.o: dofollower.cpp cfg.h doalloc.h dofollower.h dohost.h doipc.h dologs.h donet.h dopool.h dosimpletypes.h
	$(CPP) -c dofollower.cpp

doforces.o: doforces.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doforces.cpp

dohost.o: dohost.cpp cfg.h doalloc.h dohost.h doipc.h dologs.h donet.h dopool.h dosimpletypes.h
	$(CPP) -c dohost.cpp

doipc.o: doipc.cpp cfg.h doalloc.h doipc.h dologs.h
	$(CPP) -c doipc.cpp

dolayout.o: dolayout.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dolayout.cpp

doleader.o: doleader.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h doleader.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doleader.cpp

dologs.o: dologs.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h utils.h
	$(CPP) -c dologs.cpp

domap.o: domap.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c domap.cpp

domapunits.o: domapunits.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c domapunits.cpp

domouse.o: domouse.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c domouse.cpp

donet.o: donet.cpp cfg.h doalloc.h doipc.h dologs.h donet.h dopool.h dosimpletypes.h utils.h
	$(CPP) -c donet.cpp

doplayers.o: doplayers.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doplayers.cpp

doraces.o: doraces.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doraces.cpp

doschemes.o: doschemes.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doschemes.cpp

doselection.o: doselection.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doselection.cpp

dosimpletypes.o: dosimpletypes.cpp cfg.h doalloc.h dosimpletypes.h
	$(CPP) -c dosimpletypes.cpp

dosound.o: dosound.cpp cfg.h doalloc.h dologs.h dosimpletypes.h dosound.h
	$(CPP) -c dosound.cpp

dosources.o: dosources.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dosources.cpp

dounits.o: dounits.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doengine.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dounits.cpp

dowalk.o: dowalk.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h doipc.h dolayout.h dologs.h domap.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c dowalk.cpp

doworkers.o: doworkers.cpp cfg.h doalloc.h doconfig.h dodata.h dodraw.h doevents.h dofight.h dofile.h dohost.h doipc.h dolayout.h dologs.h domap.h domouse.h donet.h doplayers.h dopool.h doraces.h doschemes.h doselection.h dosimpletypes.h dosound.h dothreadpool.h dounits.h dowalk.h glfont.h glgui.h
	$(CPP) -c doworkers.cpp

glfont.o: glfont.cpp glfont.h
	$(CPP) -c glfont.cpp

glgui.o: glgui.cpp cfg.h doipc.h glfont.h glgui.h
	$(CPP) -c glgui.cpp

tga.o: tga.cpp tga.h
	$(CPP) -c tga.cpp

utils.o: utils.cpp cfg.h utils.h
	$(CPP) -c utils.cpp

DEVELOPERS = crazych index jojolase libertik martinpp peterpp

devel_stats.html: *.cpp *.h
	@echo "Running annotate..." >&2
	@cvs annotate *.h *.cpp > annotate.out 2> /dev/null
	echo "<? #### DO NOT EDIT ! #### ?>" > devel_stats.html
	echo "<? #### If you want to change this file, edit src/create_makefile.sh #### ?>" >> devel_stats.html
	echo "" >> devel_stats.html
	echo "<h2>All lines</h2>" >> devel_stats.html
	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	for developer in $(DEVELOPERS) ; do \
		echo -n "$$developer " ; \
		cat annotate.out | grep "($$developer" | sed 's/^[^:]*: //' | grep -v '^ *$$' | wc ; \
	done | sort -n -r -k 2 | \
	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:$$:</td></tr>:' >> devel_stats.html
	echo "</table>" >> devel_stats.html
	echo "" >> devel_stats.html
	echo "<h2>Lines without comments</h2>" >> devel_stats.html
	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	for developer in $(DEVELOPERS) ; do \
		echo -n "$$developer " ; \
		cat annotate.out | grep "($$developer" | sed 's/^[^:]*: //' | grep -v '^ *$$' | grep -v '^ *\(//\|/\*\|\*\)' | wc ; \
	done | sort -n -r -k 2 | \
	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:$$:</td></tr>:' >> devel_stats.html
	echo "</table>" >> devel_stats.html
	echo "" >> devel_stats.html
	echo "<h2>Comment lines</h2>" >> devel_stats.html
	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	for developer in $(DEVELOPERS) ; do \
		echo -n "$$developer" ; \
		cat annotate.out | grep "($$developer" | sed 's/^[^:]*: //' | grep '^ *\(//\|/\*\|\*\)' | wc ; \
	done | sort -n -r -k 2 | \
	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:$$:</td></tr>:' >> devel_stats.html
	echo "</table>" >> devel_stats.html
	echo "" >> devel_stats.html
	date "+<p>Last update: %Y-%m-%d %H:%M</p>" >> devel_stats.html
	sed 's/jojolase/jojolaser/' devel_stats.html > ,,devel_stats.html
	mv ,,devel_stats.html devel_stats.html
	rm annotate.out

update_web: devel_stats.html
	scp devel_stats.html user.sf.net:/home/groups/d/da/dark-oberon/htdocs/stats.php
	scp -r ../docs/documentation/html_local user.sf.net:/home/groups/d/da/dark-oberon/htdocs/documentation/
