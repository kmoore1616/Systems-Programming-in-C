echo "Example 1"
./a.out example1.txt
wc example1.txt
echo
echo "Example 2"
./a.out example2.txt
wc example2.txt
echo
echo "Both"
./a.out example1.txt example2.txt
wc example1.txt example2.txt
echo
echo "Testing 1 file..."
./a.out test.txt
wc test.txt
echo
echo "Testing long file..."
./a.out shrek.txt
wc shrek.txt
echo
echo "Testing multiple files..."
./a.out test.txt test2.txt shrek.txt
wc test.txt test2.txt shrek.txt
