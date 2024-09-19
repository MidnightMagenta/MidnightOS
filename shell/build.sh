start_time=$(date +%s%3N)

cd gnu-efi
make -j6
make bootloader
echo -e "\e[32m\e[1mBootloader built\e[0m"
cd ../kernel
make kernel
echo -e "\e[32m\e[1mKernel built\e[0m"

end_time=$(date +%s%3N)
elapsed_time=$((end_time - start_time))

echo -e "\e[32m\e[1mBuild done! Took: $elapsed_time ms\e[0m"

make buildimg
echo -e "\e[32m\e[1mImage created\e[0m"
cd ..

end_time=$(date +%s%3N)
elapsed_time=$((end_time - start_time))

echo -e "\e[32m\e[1mBuild finished! Took: $elapsed_time ms\n\e[0m"