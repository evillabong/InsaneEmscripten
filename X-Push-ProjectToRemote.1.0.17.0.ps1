
$ErrorActionPreference = "Stop"
Import-Module -Name "$(Get-Item "./Z-CoreFxs*.ps1")" -Force -NoClobber
Write-InfoDarkGray "▶▶▶ Running: $PSCommandPath"

Clear-Host
    
Clear-Host
Write-Host
Write-InfoCyan "████ Commit and Push - Git"
Write-Host

Write-Host
Write-InfoBlue "█ Commit"

Write-InfoGreen "Enter a commit message: " -NoNewLine
$message = [System.Console]::ReadLine();
git add -A
Test-LastExitCode
git status 
git commit -m "$message"

Write-Host
Write-InfoBlue "█ Push to remote"
git push

Write-Host
Write-Host
Write-InfoCyan "█ End Commit and push - Finished"