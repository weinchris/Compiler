
echo "Run testing for specific Problems"

for i in $(ls Sample/); \
do echo "${i}\n" && bin/compiler < "Sample/${i}"\
; done
