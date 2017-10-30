#!/usr/bin/python

#
# run this script with [-m model] option
# 

import YamCha
import sys

sentence = """Rockwell NNP B
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
. . O"""

c = YamCha.Chunker(sys.argv)
puts c.parse(sentence)
