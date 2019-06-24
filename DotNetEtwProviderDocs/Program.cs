using Microsoft.Diagnostics.Tracing.Parsers;
using Microsoft.Diagnostics.Tracing.Session;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Eventing.Reader;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DotNetEtwProviderDocs
{
    class Program
    {
        static string manifestsDirectory = "Manifests";

        static void DumpProvider(string providerName)
        {
            string manifestFileName = providerName + ".xml";


            string manifestFilePath = Path.Combine(manifestsDirectory, manifestFileName);

            if (File.Exists(manifestFilePath))
            {
                return;
            }

            Console.WriteLine("[+] Writing " + manifestFileName);

            string manifest = RegisteredTraceEventParser.GetManifestForRegisteredProvider(providerName);

            File.WriteAllText(manifestFilePath, manifest);
        }

        static void Main(string[] args)
        {
            if (!Directory.Exists(manifestsDirectory))
            {
                Directory.CreateDirectory(manifestsDirectory);
            }

            foreach (string providerName in EventLogSession.GlobalSession.GetProviderNames())
            {
                try
                {
                    DumpProvider(providerName);
                }
                catch (Exception)
                {
                    // Currently ignore errors
                }
            }
        }
    }
}
