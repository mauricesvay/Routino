# $Header: /home/amb/routino/xml/RCS/Makefile,v 1.8 2010/09/05 18:27:17 amb Exp $
#
# XML directory Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Web file paths

WEBDIR=../web/data

# Files to install

FILES=profiles.xml \
      translations.xml \
      tagging.xml

########

all :
	-@[ -d $(WEBDIR) ] && \
	  for file in $(FILES); do \
	     if [ ! -f $(WEBDIR)/$$file ] || [ routino-$$file -nt $(WEBDIR)/$$file ]; then \
	        echo cp routino-$$file $(WEBDIR)/$$file ;\
	        cp -f routino-$$file $(WEBDIR)/$$file ;\
	     fi ;\
	  done

########

install: all
	-[ -d $(DESTDIR)$(datadir) ] || mkdir -p $(DESTDIR)$(datadir)
	@[ -d $(DESTDIR)$(datadir) ] && \
	  for file in $(FILES); do \
	     echo cp routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	     cp -f routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
	rm -f $(WEBDIR)/*.xml

########

top=-top
include ../Makefile
