echo "Running script..."

#패치 파일을 목표 폴더에 복사합니다
cp ./patch.txt ../../../Source

#해당 폴더로 이동합니다
cd ../../../Source

#패치를 적용합니다
patch -p1 < patch.txt


#사용자의 확인입력을 기다립니다
read -p "Press Enter to Finish init process..."