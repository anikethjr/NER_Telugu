#!/usr/bin/ruby

require 'YamCha'

sentence =<<"__TMP__"
Rockwell NNP B
International NNP I
Corp. NNP I
's POS B
Tulsa NNP I
unit NN I
said VBD O
it PRP B
signed VBD O
a DT B
tentative JJ I
agreement NN I
extending VBG O
its PRP$ B
contract NN I
with IN O
Boeing NNP B
Co. NNP I
to TO O
provide VB O
structural JJ B
parts NNS I
for IN O
Boeing NNP B
's POS B
747 CD I
jetliners NNS I
. . O

__TMP__

c = YamCha::Chunker.new([$0] + ARGV);
puts c.parse(sentence)
