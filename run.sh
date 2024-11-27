make clean
make all
if [ "$1" == "net" ];then
	make run-net
else
	make run-smp
fi
make cursor
