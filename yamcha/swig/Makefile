SWIG = swig
PREFIX = YamCha

all: perl ruby python

clean:
	rm -f      *.pm *.cxx *.py
	
perl:	
	$(SWIG) -perl -shadow -c++ $(PREFIX).i
	mv -f $(PREFIX)_wrap.cxx ../perl
	mv -f $(PREFIX).pm ../perl

ruby:	
	$(SWIG) -ruby -c++ $(PREFIX).i
	mv -f $(PREFIX)_wrap.cxx ../ruby/$(PREFIX)_wrap.cpp
	
python:	
	$(SWIG) -python -shadow -c++ $(PREFIX).i
	mv -f $(PREFIX)_wrap.cxx ../python
	mv -f $(PREFIX).py ../python

java:
	$(SWIG) -java -c++ $(PREFIX).i
	mv -f $(PREFIX)_wrap.cxx ../java
	rm -fr exception.java SWIGTYPE_*.java
	mv -f *.java ../java
	
	

