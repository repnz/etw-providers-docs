# ETW Providers Docs

Windows 7 - [/Manifests-Win7-7600](/Manifests-Win7-7600)

Windows 10 - [/Manifests-Win10-1734](/Manifests-Win10-1734)

Windows provides the ETW framework for event tracing. The ETW framework comes with many built-in ETW providers, but
most of them are not documented very well. 

Using tdh.h API provider information can be dumped. For manifest based providers, a manifest can be recreated using the same method perfview uses:
(https://github.com/Kae7in/perfview/blob/444fd391db9b8275846e2a5bbb8ec1d6e73a5dad/src/PerfView/Extensibility.cs#L2523) 
(this is not the original manifest, because manifests are compiled) For non-manifest based providers, currently only keywords are dumped. But theoretically you can register to the provider and just cache
all the results from all the events (in this case the event must be raised for it to be documented)

Moreover, in a quest to find more interesting ETW providers, I started reversing specific ETW providers and document the manifests.

Currently reverse engineered providers:

- [/Manifests-Win10-1734/Microsoft-Windows-Kernel-Audit-API-Calls](/Manifests-Win10-1734/Microsoft-Windows-Kernel-Audit-API-Calls.xml)
