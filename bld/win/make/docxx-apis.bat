@echo off
pushd ..\..\..
del /F /Q doc\api\content
docxx -P -a -A -H -S -i -u -d doc\api\content -I doc\api\api.txt
popd
