# Ensure the script is run as an administrator
if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Warning "This script requires administrative privileges. Please run as Administrator."
    exit
}

# Prompt the user to select the executable file
[void][System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")
$fileDialog = New-Object System.Windows.Forms.OpenFileDialog
$fileDialog.Filter = "Executable Files (*.exe)|*.exe"
$fileDialog.Title = "Select the Executable for the Service"
if ($fileDialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
    $exePath = $fileDialog.FileName
} else {
    Write-Host "No file selected. Exiting script."
    exit
}

$serviceName = "VoicemeeterOpti"

# Create the service using sc.exe
try {
    $createServiceCommand = "sc create `"$serviceName`" binPath= `"$exePath`" start= auto"
    Invoke-Expression $createServiceCommand
    Write-Host "Service '$serviceName' created successfully with the executable at '$exePath'."
    $addDescriptionCommand = "sc description $serviceName 'Autostarts the programm to remove the Voicemeeter waiting time'"
    Invoke-Expression $addDescriptionCommand
    Write-Host "Description successfully applied."
} catch {
    Write-Error "Failed to create the service: $_"
}

# To delete :
# sc delete VoicemeeterOpti