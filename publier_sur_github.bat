@echo off
chcp 65001 >nul
echo ============================================
echo   GhostSurf -- Publier sur GitHub
echo ============================================
echo.
echo Ce script envoie le code sur GitHub.
echo GitHub compilera le plugin automatiquement (gratuit).
echo.

cd /d "%~dp0"

REM Verifier Git
where git >nul 2>&1
if errorlevel 1 (
    echo Installation de Git...
    winget install --id Git.Git -e --accept-package-agreements --accept-source-agreements
    set "PATH=%PATH%;C:\Program Files\Git\cmd"
)

echo.
echo Etape 1 : Initialisation Git...
git init
git add .
git commit -m "GhostSurf VST3 plugin - initial commit"

echo.
echo ============================================
echo   MAINTENANT :
echo ============================================
echo.
echo 1. Va sur https://github.com/new
echo 2. Cree un repo nomme "ghostsurf" (public ou prive)
echo 3. NE coche pas "Add README"
echo 4. Copie l'URL du repo (ex: https://github.com/tonpseudo/ghostsurf.git)
echo.
set /p REPO_URL="Colle l'URL ici : "

git remote add origin %REPO_URL%
git branch -M main
git push -u origin main

echo.
echo ============================================
echo   CODE ENVOYE !
echo ============================================
echo.
echo GitHub compile maintenant automatiquement.
echo.
echo Pour telecharger le plugin :
echo 1. Va sur %REPO_URL%
echo 2. Clique sur "Actions" (onglet en haut)
echo 3. Attends ~10 minutes que le build finisse
echo 4. Clique sur le build termine
echo 5. Telechargez "GhostSurf-VST3-Windows.zip"
echo 6. Extrais GhostSurf.vst3 dans :
echo    C:\Program Files\Common Files\VST3\
echo 7. Rescan dans FL Studio
echo.
pause
