上传代码到GitHub：

0.git init (第一次执行即可)
1.git add -A
2.git commit -m "v1, disable_watch_dog、set_clock、initialize_sdram(memsetup)、relocate(nand_init、copy_code_to_sdram、clean_bss)."
3.git tag v1
4.git remote add origin https://github.com/hujinfan/my_bootloader.git （第一次执行即可）
5.git push -u origin master
6.git push origin --tags
