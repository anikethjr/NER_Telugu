use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'CC'                => 'g++',
    'LD'                => 'g++',
    'NAME'		=> 'YamCha',
    'VERSION_FROM'	=> 'YamCha.pm', # finds $VERSION
    'INC'               => '-I/usr/local/include',
    'LIBS'              => '-L/usr/local/lib  -lm -lstdc++ -lyamcha',
    'OBJECT'            => 'YamCha_wrap.o' 
);
