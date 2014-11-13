#!/usr/bin/bash
test_dir=/mnt/japan
file=$(find $test_dir -regex '.*\.\(mkv\|mp4\|avi\|mkv\|mp3\)'| sort -R | head -n 1)
echo $file

echo -e "#!/usr/bin/bash\n./audio \"$file\"" > last_test.sh

./audio "$file"
