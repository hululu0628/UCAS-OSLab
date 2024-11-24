make clean
make all
if [ "$1" == "smp" ];then
	make run-smp
else
	make run
fi
make cursor
