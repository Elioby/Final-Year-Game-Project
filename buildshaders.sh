shaderc="../Libraries/bgfx/.build/win32_vs2017/bin/shadercDebug.exe"

for f in Project/shadersrc/*varying.sc; do
	f=${f##*/}; 
	f="$(echo "$f" | rev | cut -d'_' -f2- | rev)";
	echo "Building $f shader..."

	$shaderc -f Project/shadersrc/"$f"_vs.sc -o Project/res/shader/"$f".vs --type v --platform windows --varyingdef Project/shadersrc/"$f"_varying.sc
	$shaderc -f Project/shadersrc/"$f"_fs.sc -o Project/res/shader/"$f".fs --type f --platform windows --varyingdef Project/shadersrc/"$f"_varying.sc
done