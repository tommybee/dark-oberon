# OPTIONS

UNIX='yes' # yes/no
SOUND='0'  # 1/0
DEBUG='0'  # 1/0

##############################################################################

CPPFLAGS='-g -Wall -O'
TARGETS='../dark-oberon'
INCLUDES='-I/usr/X11R6/include -I/usr/X11R6/include/GL -I../libs'
LIBPATHS='-L/usr/X11R6/lib -L/usr/lib -L/usr/local/lib -L../libs'
LIBRARIES='-pthread -lglfw -lGL -lX11 -lGLU'
CPP='g++ $(CPPFLAGS) $(INCLUDES) $(DEFINES)'
DATA_DIR=''

##############################################################################

e='' # empty
n='
' # new line

# defines
DEFINES="-DDATA_DIR='\"$DATA_DIR\"'"

test "$UNIX" = "yes" && DEFINES="$DEFINES -DUNIX=1"
DEFINES="$DEFINES -DSOUND=\$(SOUND) -DDEBUG=\$(DEBUG)"

# find out objects files
OBJECTS=`eval "ls *.cpp | sed s/cpp$/o/"`
OBJECTS=`echo $OBJECTS`

cat > Makefile <<-END
	SOUND ?= $SOUND
	DEBUG ?= $DEBUG

	CPP = $CPP
	CPPFLAGS = $CPPFLAGS
	DEFINES = $DEFINES
	INCLUDES = $INCLUDES
	LIBPATHS = $LIBPATHS
	LIBRARIES = $LIBRARIES
	OBJECTS = $OBJECTS
	TARGETS = $TARGETS

	all: tags $TARGETS checking

	checking: .checking
	.checking: *.h *.cpp
	$e	@echo Checking format of source files...
	$e	@if egrep -l "\`printf '\t'\`|\`printf '\r'\`" *.cpp *.h; then echo "Error: Previous listed files contain illegal characters (tabs or ^M)"; false; fi
	$e	@echo OK
	$e	@touch .checking

	clean: 
	$e	rm -f \$(OBJECTS) \$(TARGETS) *core core.* tags
	
	tags: *.h *.cpp
	$e	-exctags * 2> /dev/null || ctags * 2> /dev/null

	docs:
	$e	rm -rf ../docs/documentation/html_local
	$e	doxygen .doxygen.conf 2>&1 | tee .doxygen.log
END

# create targets for TARGETS
for i in $TARGETS ;do
	cat >> Makefile <<-END

		$i: \$(OBJECTS)
		$e	\$(CPP) \$(OBJECTS) \$(LIBPATHS) \$(LIBRARIES) -o $i
	END
done

# create object files targets (.o) for all *.cpp files
for i in *.cpp ; do
	includes=`grep '#include "' "$i" | sed 's/.*"\(.*\)".*/\1/'`
	oldincludes=""

	# we need all includes also recursively
	echo "Searching recursive includes for $i..."
	while [ "$includes" != "$oldincludes" ] ;do
		oldincludes="$includes"
		for j in $includes; do
			eval saved="\$${j%.h}"
			if [ "$saved" ] ;then
				subincludes="$saved"
			else
				subincludes=`grep '#include "' "$j" | sed 's/.*"\(.*\)".*/\1/'`
				eval ${j%.h}="\"$subincludes$n\""
			fi
			includes=`echo "$includes" ; echo "$subincludes"`
		done
		includes=`echo "$includes" | sort -u`
	done

	# add every include to dependencies
	dependencies=''
	for j in $includes ;do
		dependencies="$dependencies $j"
	done

	# add also actual file to dependencies
	dependencies="$i$dependencies"

	cat >> Makefile <<-END

		${i%.cpp}.o: $dependencies
		$e	\$(CPP) -c $i
	END
done

cat >> Makefile <<-END

	DEVELOPERS = crazych index jojolase libertik martinpp peterpp

	devel_stats.html: *.cpp *.h
	$e	@echo "Running annotate..." >&2
	$e	@cvs annotate *.h *.cpp > annotate.out 2> /dev/null
	$e	echo "<? #### DO NOT EDIT ! #### ?>" > devel_stats.html
	$e	echo "<? #### If you want to change this file, edit src/create_makefile.sh #### ?>" >> devel_stats.html
	$e	echo "" >> devel_stats.html
	$e	echo "<h2>All lines</h2>" >> devel_stats.html
	$e	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	$e	for developer in \$(DEVELOPERS) ; do \\
	$e		echo -n "\$\$developer " ; \\
	$e		cat annotate.out | grep "(\$\$developer" | sed 's/^[^:]*: //' | grep -v '^ *\$\$' | wc ; \\
	$e	done | sort -n -r -k 2 | \\
	$e	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:\$\$:</td></tr>:' >> devel_stats.html
	$e	echo "</table>" >> devel_stats.html
	$e	echo "" >> devel_stats.html
	$e	echo "<h2>Lines without comments</h2>" >> devel_stats.html
	$e	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	$e	for developer in \$(DEVELOPERS) ; do \\
	$e		echo -n "\$\$developer " ; \\
	$e		cat annotate.out | grep "(\$\$developer" | sed 's/^[^:]*: //' | grep -v '^ *\$\$' | grep -v '^ *\(//\|/\*\|\*\)' | wc ; \\
	$e	done | sort -n -r -k 2 | \\
	$e	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:\$\$:</td></tr>:' >> devel_stats.html
	$e	echo "</table>" >> devel_stats.html
	$e	echo "" >> devel_stats.html
	$e	echo "<h2>Comment lines</h2>" >> devel_stats.html
	$e	echo "<table class=box><tr><th></th><th>lines</th><th>words</th><th>chars</th><tr>" >> devel_stats.html
	$e	for developer in \$(DEVELOPERS) ; do \\
	$e		echo -n "\$\$developer" ; \\
	$e		cat annotate.out | grep "(\$\$developer" | sed 's/^[^:]*: //' | grep '^ *\(//\|/\*\|\*\)' | wc ; \\
	$e	done | sort -n -r -k 2 | \\
	$e	sed 's:  *:</td><td>:g' | sed 's:^:<tr><td>:' | sed 's:\$\$:</td></tr>:' >> devel_stats.html
	$e	echo "</table>" >> devel_stats.html
	$e	echo "" >> devel_stats.html
	$e	date "+<p>Last update: %Y-%m-%d %H:%M</p>" >> devel_stats.html
	$e	sed 's/jojolase/jojolaser/' devel_stats.html > ,,devel_stats.html
	$e	mv ,,devel_stats.html devel_stats.html
	$e	rm annotate.out

	update_web: devel_stats.html
	$e	scp devel_stats.html user.sf.net:/home/groups/d/da/dark-oberon/htdocs/stats.php
	$e	scp -r ../docs/documentation/html_local user.sf.net:/home/groups/d/da/dark-oberon/htdocs/documentation/
END
