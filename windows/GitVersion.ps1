Param ([String]$filename = "..\..\Version.h")
# Build a version header using the git describe command
set-alias git "C:\Program Files\Git\cmd\git.exe"
$filename = Join-Path $pwd $filename
$define = "#define GIT_VERSION "
$gitdesc = git describe --dirty='-derive'
echo $define`"$gitdesc`" | Out-File -FilePath $filename
# SIG # Begin signature block
# MIIFuQYJKoZIhvcNAQcCoIIFqjCCBaYCAQExCzAJBgUrDgMCGgUAMGkGCisGAQQB
# gjcCAQSgWzBZMDQGCisGAQQBgjcCAR4wJgIDAQAABBAfzDtgWUsITrck0sYpfvNR
# AgEAAgEAAgEAAgEAAgEAMCEwCQYFKw4DAhoFAAQUwaZMMecnwsldfF5BlGNXzbP7
# WxigggNCMIIDPjCCAiqgAwIBAgIQ+/i1WksVkphGndMf8YuIaDAJBgUrDgMCHQUA
# MCwxKjAoBgNVBAMTIVBvd2VyU2hlbGwgTG9jYWwgQ2VydGlmaWNhdGUgUm9vdDAe
# Fw0xNjAxMjgxMzQ5NTNaFw0zOTEyMzEyMzU5NTlaMBoxGDAWBgNVBAMTD1Bvd2Vy
# U2hlbGwgVXNlcjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOxnNFe4
# 0ZTbUz5A4TUlmug7dJp0ZvmSkJ7ArnKIF9btkj+xnSTNXheg57sl7fsVThSMJSsc
# cDSPwMW2iHkclUS8enmPRzKkvM3j0riMQXgG8DCMuN2pvHEFWwh+vFJRahW/ykfg
# NCeYPLhw2U+C3VaHNRdz9F9zDR+BdO6E/+Yy7swr/FeOdajldnIasdp4obVSgnBX
# VG7i7HGxEvN/LZpctKKhRRzkQAv1XzIunmBLsPpzknFqjJSgx8HxIoZB6ViPm6Ct
# 2K/510KQ2nxETDSzpsKMG/HxJFdJDLiV4KJZIO8e36JFLmZE6x7cq7MUAZTnlIGl
# VlthY25nDyDpDJcCAwEAAaN2MHQwEwYDVR0lBAwwCgYIKwYBBQUHAwMwXQYDVR0B
# BFYwVIAQ69jNQO5IdSJz1Z8Ejx8ieKEuMCwxKjAoBgNVBAMTIVBvd2VyU2hlbGwg
# TG9jYWwgQ2VydGlmaWNhdGUgUm9vdIIQh8gFdmB8nbZMoNg7bwVywjAJBgUrDgMC
# HQUAA4IBAQANQ70eNVZ0SN4jb7O7M2pgCvSHxaTBrQ2UC2D2k1V+q8mJmIsgOS1A
# 72kTVsr1683kDBRzlbZFReK384wBqDP1vZUDD0+WygEclxf18TJ5Sx7M3W8Wpa3M
# L3taqVTVMIwlqyap3DbUJpQCrnxyulyWS84073fNGU99A6iZxTB6oC9tYSZClg0a
# 6HAQs38DEzBypjWtpE6lgnPgPx5Ua4HZUIFYBJf1heAd1ELreLeFLgbzUuSDhHeu
# OILEJupku0MEfEtbUouhbo8U4ARq4qlzrv0y5MuQa6EutVqQ0LJWHC4eLp+dqDl+
# zkXOWBkYc0m8Xe/84Pv3adslB5N3UyDtMYIB4TCCAd0CAQEwQDAsMSowKAYDVQQD
# EyFQb3dlclNoZWxsIExvY2FsIENlcnRpZmljYXRlIFJvb3QCEPv4tVpLFZKYRp3T
# H/GLiGgwCQYFKw4DAhoFAKB4MBgGCisGAQQBgjcCAQwxCjAIoAKAAKECgAAwGQYJ
# KoZIhvcNAQkDMQwGCisGAQQBgjcCAQQwHAYKKwYBBAGCNwIBCzEOMAwGCisGAQQB
# gjcCARUwIwYJKoZIhvcNAQkEMRYEFP+q73bQU7qS5ZwJIkuon/VaJDgeMA0GCSqG
# SIb3DQEBAQUABIIBAGlg3+TUPWOox3vRwv9XhrqBEASfuIPiLJZi5gb9PVsEnVOO
# /oMoPGEZyjLqb2R5ur6fThZZm0/Ag46fKPUWUDCe5+WHIOw5M+/RILf6me3Zf0Ac
# OcXOGrT5pi8VXhYgRoDDKlU5rRStFqje29/nJMBg1wqKkbozCoH46XFn8+/xm5Jq
# vpAolkHj3w2wVBnB7mSiUokMdVy0gUchSDA8GMBWYPo7rkEwKTjXr9pI0cZsp3z6
# ESry1khZrat7HDNGCrlWn1+ZtOmS50uhFgXXsXdsut5DSNYAcyapuZlXLIISL3f4
# mWTeTIJxUmkn46G+38rkr81k0duMiZot9elYljI=
# SIG # End signature block