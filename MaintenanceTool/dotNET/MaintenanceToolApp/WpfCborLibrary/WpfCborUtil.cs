using PeterO.Cbor;
using System.Collections.Generic;

namespace WpfCborLibrary
{
    public class WpfCborUtil
    {
        public static IList<CBORObject> ConvertCBORObjectToIList(CBORObject imageArray)
        {
            return imageArray.AsList();
        }
    }
}
