require 'mkmf'

have_header('yamcha.h')

' -lm -lstdc++'.split.each do | lib |
  lib.gsub!(/^-l/,  "")
  lib.gsub!(/^\s*/, "")  
  lib.gsub!(/\s*$/, "")
  have_library(lib)
end

have_library('yamcha') 

create_makefile('YamCha')
