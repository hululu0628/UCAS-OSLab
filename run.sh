make clean
make all
cp ./build/image ./build/image2
if [ "$1" == "net" ];then
	make run-net
else
	make run-smp
fi
make cursor
