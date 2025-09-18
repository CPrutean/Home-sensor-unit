# generate_certs.py

import datetime
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
import dotenv
import os

from flask.cli import load_dotenv


def generate_self_signed_cert(hostname, key_file="key.pem", cert_file="cert.pem"):
    """
    Generates a self-signed certificate and a private key for local development.
    """
    # 1. Generate a private key
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
    )
    load_dotenv()
    # 2. Define the certificate's subject and issuer
    subject = issuer = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, os.getenv("CERT_COUNTRY")),
        x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, os.getenv("CERT_STATE")),
        x509.NameAttribute(NameOID.LOCALITY_NAME, os.getenv('CERT_LOCALITY')),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, os.getenv('CERT_ORGANIZATION_NAME')),
        x509.NameAttribute(NameOID.COMMON_NAME, hostname),
    ])

    # 3. Build and sign the certificate
    cert_builder = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(private_key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
        .not_valid_after(
            # Certificate is valid for 1 year
            datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(days=365)
        )
        .add_extension(
            # Use Subject Alternative Name (SAN) for modern browser compatibility
            x509.SubjectAlternativeName([x509.DNSName(hostname)]),
            critical=False,
        )
    )

    certificate = cert_builder.sign(private_key, hashes.SHA256())

    # 4. Write the private key to a file
    with open(key_file, "wb") as f:
        f.write(
            private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption(),
            )
        )
    print(f"✅ Private key saved to {key_file}")

    # 5. Write the certificate to a file
    with open(cert_file, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.PEM))
    print(f"✅ Certificate saved to {cert_file}")


if __name__ == "__main__":
    # Generate files for localhost development
    generate_self_signed_cert("localhost")