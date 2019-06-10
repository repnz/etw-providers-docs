using Microsoft.Diagnostics.Tracing.Parsers;
using Microsoft.Diagnostics.Tracing.Session;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DotNetEtwProviderDocs
{
    class Program
    {
        static string manifestsDirectory = "Manifests";

        static void DumpProvider(Guid guid)
        {
            string manifestFileName;

            try
            {
                manifestFileName = TraceEventProviders.GetProviderName(guid) + ".xml";
            }
            catch (Exception)
            {
                manifestFileName = guid.ToString() + ".xml";
            }
            
            string manifestFilePath = Path.Combine(manifestsDirectory, manifestFileName);

            if (File.Exists(manifestFilePath))
            {
                return;
            }

            Console.WriteLine("[+] Writing " + manifestFileName);

            string manifest = RegisteredTraceEventParser.GetManifestForRegisteredProvider(guid);

            File.WriteAllText(manifestFilePath, manifest);
        }

        static void Main(string[] args)
        {

            if (!Directory.Exists(manifestsDirectory))
            {
                Directory.CreateDirectory(manifestsDirectory);
            }

            foreach (Guid providerGuid in TraceEventProviders.GetRegisteredOrEnabledProviders())
            {
                try
                {
                    DumpProvider(providerGuid);
                }
                catch (Exception)
                {
                    // Currently ignore errors
                }
            }
        }
    }
}
