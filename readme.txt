�ϴ����뵽GitHub��

0.git init (��һ��ִ�м���)
1.git add -A
2.git commit -m "v1, disable_watch_dog��set_clock��initialize_sdram(memsetup)��relocate(nand_init��copy_code_to_sdram��clean_bss)."
3.git tag v1
4.git remote add origin https://github.com/hujinfan/my_bootloader.git ����һ��ִ�м��ɣ�
5.git push -u origin master
6.git push origin --tags

�汾2 
1.git add -A
2.git commit -m "v2, add boot.c for main()"
3.git tag v2
4.git push origin master
5.git push origin --tags

�汾3
1.git add -A
2.git commit -m "v3, modify the program error and compile pass"
3.git tag v3
4.git push origin master
5.git push origin --tags

�汾4
1.git add -A
2.git commit -m "v4, bootloader run success"
3.git tag v4
4.git push origin master
5.git push origin --tags

�汾5
1.git add -A
2.git commit -m "v5, add print info. use the puthex()"
3.git tag v5
4.git push origin master
5.git push origin --tags

�汾6
1.git add -A
2.git commit -m "v6, improve the code to boot faster(increase cpu frequency and enable icache)"
3.git tag v6
4.git push origin master
5.git push origin --tags