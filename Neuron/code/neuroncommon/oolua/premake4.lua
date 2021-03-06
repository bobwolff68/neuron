--oolua premake4 file

dofile("helper4.lua")

if _ACTION == "clean" then
  os.rmdir("obj")
  os.rmdir("bin")
end


newaction {
   trigger     = "vs2010",
   description = "Generate Microsoft Visual Studio 2010 project files",
   execute = function ()
				--files in build_scrips need to be run from that directory
				os.execute("\"cd build_scripts&&premake_vs2010.bat\"")
   end
}

newaction {
   trigger     = "vs2010_clean",
   description = "Remove all binaries and generated files generated by VS 2010",
   execute = function ()
				--files in build_scrips need to be run from that directory
				os.execute("\"cd build_scripts&&cleanVS10.bat\"")
   end
}

solution("oolua")
   configurations { "Debug", "Release" }


dofile("./oolua4.lua")
dofile("./unit_tests/test.unit.lua")
include("./unit_tests/tests_may_fail/")
include("./file_generator/")
include("./profile/")
include("./unit_tests/string_is_integral/")
dofile("./unit_tests/test.unit.exceptions.lua")

