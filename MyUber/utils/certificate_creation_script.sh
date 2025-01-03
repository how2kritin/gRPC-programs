#!/bin/bash

CERT_DIR=$1
CA_KEY=${CERT_DIR}"/ca.key"
CA_CERT=${CERT_DIR}"/ca.crt"
SERVER_IP="localhost"
CLIENT_NAME=$2
CLIENT_ROLE=$3

openssl genpkey -algorithm RSA -out "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.key" -pkeyopt rsa_keygen_bits:2048
openssl req -new -key "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.key" \
    -out "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.csr" \
    -subj "/CN=${CLIENT_NAME}_${CLIENT_ROLE}" \
    -addext "subjectAltName=DNS:${SERVER_IP}"
openssl x509 -req -in "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.csr" \
    -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial \
    -out "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.crt" \
    -days 365 -set_serial 01 -sha256 \
    -extfile <(printf "subjectAltName=DNS:${SERVER_IP}")

rm "${CERT_DIR}/${CLIENT_NAME}_${CLIENT_ROLE}.csr"
#rm "${CERT_DIR}/ca.srl"