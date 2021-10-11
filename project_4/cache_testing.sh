mkdir -p files
python3 file_generator.py

echo "Without cache" >> out_file
./simplefs test_disk 200 0 < commands | grep "disk block" | cat > out_file
echo "" >> out_file
echo "Cache size - 5" >> out_file
./simplefs test_disk 200 5 < commands | grep "disk block" | cat >> out_file
echo "" >> out_file
echo "Cache size - 10" >> out_file
./simplefs test_disk 200 10 < commands | grep "disk block" | cat >> out_file
echo "" >> out_file
echo "Cache size - 12" >> out_file
./simplefs test_disk 200 12 < commands | grep "disk block" | cat >> out_file
echo "" >> out_file

rm -rf files
rm temp1
rm temp2

