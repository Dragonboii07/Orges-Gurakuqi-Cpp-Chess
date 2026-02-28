@echo off
if not exist "bin\Debug" (
  mkdir "bin\Debug"
)

echo Building project...
g++ -g -I"include" -o "bin\Debug\project.exe" "src\main.cpp" "src\person.cpp" "src\student.cpp" "src\teacher.cpp"
if errorlevel 1 (
  echo.
  echo Build failed.
  pause
  exit /b 1
)

echo Running project...
"bin\Debug\project.exe"
echo.
echo Done.
pause
