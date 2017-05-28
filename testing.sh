
echo "Run testing for specific Problems"

for i in $(ls Sample/); \
do echo "${i}" && bin/compiler < "Sample/${i}" && echo \
; done
