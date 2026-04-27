# FIMS – Funnel Indexed Mesh Storage

**FIMS** is a high-performance, information-theoretic decentralized Content Delivery Network (CDN) and peer-to-peer data mesh. It delivers **low‑latency data streaming** while ensuring that participants (infrastructure nodes and end‑users) are mathematically protected from unauthorised network surveillance and bulk traffic monitoring.

Unlike traditional anonymity networks that sacrifice speed to hide *where* data travels, FIMS decouples content representation from the transport layer: intermediate nodes never require access to the plaintext payload. The network operates as a **Distributed Linear Mesh**, entangling data into localised algebraic equations over Galois Field 2 (GF(2)).

---

## ⚡ The Mathematics of Speed

Standard privacy networks trade throughput for security. FIMS trades *representation* for security. By shifting the cryptographic burden from multi‑hop routing to local, vectorised CPU instructions, FIMS achieves line‑rate throughput.

1.  **Direct P2P Throughput:** Data flows straight from a source node to the requesting node. There are no intermediate relays or bandwidth‑limiting hops.
2.  **Storage Layout:** All files are padded, encrypted using AES, and then concatenated with the decryption key and hash. The final payload format is: HASH | KEY | ENCRYPTED_DATA. Then it is split in 3 xor shards deterministically based on data hash. Those shards are merged with existing blobs.
3.  **Seeding:** The receiver asks for a list of available blobs and then requests the seeder to transmit a specific XORed sum of them. Thus, no extra bandwidth is wasted.
4.  **Receiving:** The receiver gathers these XORed transmissions from multiple seeders and XORs them a final time against their own local blobs and shards to recover the final file.

---

## 🧮 Clustering: Dynamic Vector Spaces

To keep the algebraic problem local and fast, FIMS partitions the global dataset into mathematical **Clusters**.

### How Clusters Are Formed
1.  **Hash Mapping:** Every data object possesses an 8-bit unsigned integer $L(F)$ representing length, and $H(F)$ a 256‑bit content hash. Getting index key as L | H
2.  **Deterministic Prefixing:** A Cluster ID is derived from the first $k$ bits of that index key. For example, with $k=12$, the cluster `0x7A2` contains all objects whose hashes share those 12 bits.
3.  **Locality Rule:** Infrastructure nodes in a given cluster only generate and store coded blocks using source vectors that belong to that exact same cluster and shard ID.

### Adaptive Density (Self‑Balancing)
To maintain strong statistical privacy, the prefix length $k$ self‑adjusts automatically across the network:
*   **Low density ($N < 100$ active files):** The cluster is too thin, raising the risk of statistical inference. The network shortens the prefix, merging clusters to increase the anonymity set.
*   **High density ($N > 1\,000$ active files):** Decoding becomes unnecessarily heavy. The network lengthens the prefix, splitting the cluster.
*   **Result:** Every transfer operates inside a well‑populated, mathematically optimal anonymity set that provides robust privacy.

---

## 🧬 Algebraic Reconstruction

In FIMS, an infrastructure node does not host original files. It stores **blobs** ($B$)-linear combinations of shards ($S$) from several source objects in the same Cluster.

### Stored Blocks
*  **Blob:** A blob is defined as:  
$$B_i = S_{C|a, i} \oplus S_{C|b, i} \oplus S_{C|c, i} \oplus S_{C|d, i}$$
  Where the coefficients $a,b,c,d$ are distinct hash suffixes of files in $C$ Cluster. $B$ is a blob of that Cluster with $i \in 1,2,3$ being shard index. There can be 3 to 4 shards in a blob.
*  **Blob Hash:** Each blob has a Cluster and a xor of $a,b,c,d$ as their hash.


### Reconstruction of Receipt

The receiver:
0. Search of target file hash.
1. Fetches availible blob's hashes from infrastructure nodes via direct P2P requests.
2. Solves linear equation locally to determine which nodes to ask for which blobs.
3. XORs the incoming blobs with the local blobs and shards, recovering the original hash | key | encryption via the involution property of XOR.
4. Pipes the resulting chunk directly into the target application.

Because the infrastructure node only stores opaque linear combinations and never receives the receipt, it operates with **Information-Theoretic Security (ITS)**.

---

## 🛡️ Comprehensive Privacy Architecture

FIMS protects privacy at every layer:

### 1. Storage Privacy (Content‑Oblivious Hosting)
Infrastructure nodes enforce a **Threshold Hosting Rule**: Only shares one shard of any file at a time. Because the shared data mathematically cannot be solved without two other shards, nodes possess high deniability.

### 2. Private Information Retrieval (PIR)
Receivers request coded blocks through **blinded matrix queries**. Instead of asking for a single blob $B_i$, they request an XOR sum of several blobs. This way seeder doesn't know what files were taken out.

### 3. Query Privacy (Obfuscated DHT Lookups)
Hash‑table queries are protected using **obfuscated Bloom filters**. The receiver constructs a filter containing the target hash alongside calibrated synthetic noise (decoys). DHT nodes return all matching index keys and descriptions; the receiver silently discards the decoy results locally. The network index never learns the user's true intent.

### 4. Forensic Volatility (Stream‑Only Consumption)
Reconstructed content is processed entirely in memory (RAM) and is never written to non‑volatile storage. This ensures zero persistent forensic footprint on the user's hardware, minimises SSD wear, and enables instantaneous media playback.

---

## 🎭 Privacy vs Performance
To reconstruct any file in FIMS, a receiver must obtain three mathematically independent shards ($S_1, S_2, S_3$). Because nodes are restricted to hosting only one shard per file (one blobs type), the receiver must coordinate a multi-node download. 

The baseline bandwidth cost for a "Light" transfer is **3x the file size**. Users can opt to increase this multiplier to inject algebraic noise and decoys, scaling their privacy from "Content-Oblivious" to "Statistically Invisible."

### 📊 Privacy vs. Performance

| Level | Request Strategy | Bandwidth Cost | Anonymity Strength |
| :--- | :--- | :--- | :--- |
| **Light** | Fetch 3 specific shards from 3 nodes. | **3x** | **Content-Oblivious:** No node knows what they are hosting or what you are building. |
| **Medium** | Fetch 3 blinded sums (Target + 1 Decoy) from 6 nodes. | **6x** | **Query Privacy:** Nodes cannot distinguish the target shard from the decoy shard. |
| **Heavy** | Solve a large mesh of sums (Target + 3+ Decoys) from 12+ nodes. | **12x+** | **Statistical Anonymity:** Traffic patterns are masked by high-entropy algebraic noise. |

---

## 💡 The Contributor Bonus (Bandwidth Reduction)
If you are an active node and already store shards locally, your network bandwidth requirement drops significantly. The "Light" 3x cost is reduced based on your local holdings:
*   **Have 1 shard locally:** Fetch 2 shards ($2x$ bandwidth).
*   **Have 2 shards locally:** Fetch 1 shard ($1x$ bandwidth).
*   **Result:** The more you contribute to the mesh, the faster and more private your own downloads become, as the "missing variables" needed to solve the file are never requested from the network.

---

## 🔎 Funnel Indexed (Bit-Funnel Search)
FIMS implements a distributed version of the **BitFunnel** algorithm (originally developed by Bing) to manage high-speed discovery across the mesh:
*   **Signature Scouting:** Requester broadcasts bloom filter with noise.
*   **Others Inverted Indexes:** Each node performs searches within its own local inverted index and returns documents that match bloom filter request.
*   **Local Inverted Index::** Once we got document metadata reciever simply searches in Local Inverted Index.
*   **Efficiency:** This avoids the overhead of traditional DHT-only searches, providing high-performance search capabilities while maintaining the privacy of the underlying data mesh.

---

## 📦 Prove of Space (64 GB)
A minimum of **64 GB** of free space is required to support prove of space. This capacity requirement ensures Sybil resistance and validates that nodes contribute meaningful storage to the mesh. The allocated space is filled with verifiable, pre-computed Galois Field combinations (blobs) that the node must continuously prove it retains via periodic cryptographic challenges.

---

## 📜 License

FIMS is licensed under the **GNU General Public License v3.0** (GPLv3).  
See the [LICENSE](LICENSE) file for details.

---

## ⚖️ Disclaimer

FIMS is a general‑purpose protocol for high‑speed, privacy‑preserving data distribution and content‑oblivious storage. It is engineered to protect the fundamental right to privacy through applied information theory. The developers, contributors, and maintainers do not endorse, facilitate, or encourage any use of the software for illegal acts. All users operate the software at their own risk and must comply with the laws of their local jurisdiction.