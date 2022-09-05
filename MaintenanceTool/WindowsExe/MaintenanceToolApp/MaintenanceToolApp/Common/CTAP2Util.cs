namespace MaintenanceToolApp.Common
{
    internal class KeyAgreement
    {
        public int Kty { get; set; }
        public int Alg { get; set; }
        public int Crv { get; set; }
        public byte[] X { get; set; }
        public byte[] Y { get; set; }

        public KeyAgreement()
        {
            X = new byte[0];
            Y = new byte[0];
        }

        public KeyAgreement(int kty, int alg, int crv, byte[] x, byte[] y)
        {
            Kty = kty;
            Alg = alg;
            Crv = crv;
            X = new byte[32];
            Y = new byte[32];
            x.CopyTo(X, 0);
            y.CopyTo(Y, 0);
        }
    }
}
