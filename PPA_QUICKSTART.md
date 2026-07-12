# Launchpad PPA クイックスタートガイド

**game_ryo様用高速セットアップガイド**

このドキュメントで、Flashcard を Launchpad PPA で公開するための最小限の手順をご推奨いたします。

## 不可欠情報

| 項目 | 追記 |
|------|--------|
| **Launchpad ユーザー名** | (`username`) の部分 |
| **GPG Key ID** | `gpg --list-secret-keys` で確認 |
| **Git ユーザー** | game_ryo 等 |
| **Git メール** | game_ryo@ryopc.dev 等 |

---

## 複旧化注意⚠️

以下をすでに実行済みか確認：

- [ ] GPG キーを作成済み (Step 1.1)
- [ ] GPG キーを Launchpad に登録済み (Step 1.3)
- [ ] PPA を Launchpad で作成済み (Step 2)
- [ ] GitHub Secrets を設定済み (Step 8)

上記が完了したら、速やかに監管を成功させることでて斉ぜようはみまう。

---

## Step 1: GPG キーペア（初回のみ）

### 1.1 キーを作成

```bash
gpg --full-gen-key
```

尹の設定：
- **Key type**: 1 (RSA and RSA)
- **Key size**: 4096
- **Validity**: 0 (無期限)
- **Name**: game_ryo
- **Email**: game_ryo@ryopc.dev
- **Passphrase**: 安全なパスフレーズ

### 1.2 キー ID を確認

```bash
gpg --list-secret-keys --keyid-format LONG

# 出力例：
# sec   rsa4096/XXXXXXXXXXXXXXXX 2026-07-12 [SC]
#       YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
# uid                 [ultimate] game_ryo <game_ryo@ryopc.dev>

# YYYYYY... の部分が GPG Key ID
KEY_ID="YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY"
```

### 1.3 Launchpad サーバーに送信

```bash
gpg --send-keys --keyserver keyserver.ubuntu.com $KEY_ID
```

### 1.4 Launchpad で登録

1. https://launchpad.net/~/+editpgpkeys にアクセス
2. 上記の `KEY_ID` を入力
3. 「Import Key" をクリック
4. メール確認を完了

---

## Step 2: PPA を Launchpad で作成

1. https://launchpad.net/~username/+ppas にアクセス（`username` は自身のユーザー名）
2. 「Create a new PPA" をクリック
3. **PPA Name**: `flashcard`
4. **Description**: `Lightweight cross-platform flashcard application built with C and Raylib`
5. 「Create PPA" をクリック

完了后、以下 URL が表示されます：
```
https://launchpad.net/~username/+archive/ubuntu/flashcard
```

---

## Step 3: GitHub Secrets を設定

### 3.1 GitHub で設定

1. https://github.com/ryopc/c-flashcard-gui/settings/secrets/actions にアクセス
2. 「New repository secret" をクリック
3. 以下を作成：

| Secret Name | Value |
|---|---|
| `GPG_KEY_ID` | Step 1.2 で確認した KEY_ID |
| `LAUNCHPAD_USERNAME` | Launchpad ユーザー名 |
| `GIT_EMAIL` | game_ryo@ryopc.dev |
| `GIT_NAME` | game_ryo |
| `GPG_PRIVATE_KEY` | 下記を参照 |
| `GPG_PASSPHRASE` | GPG キーのパスフレーズ |

### 3.2 `GPG_PRIVATE_KEY` を取得

```bash
# GPG private key を Base64 エンコード
# (コピーして GitHub Secrets に貼り付け)
gpg --export-secret-key $KEY_ID | base64 -w 0
```

---

## Step 4: タグを作成して公開

### 4.1 GitHub でタグを作成

```bash
# ローカルで changelog を更新
cd c-flashcard-gui
dch -i

# テキストエディタで新管特を記入：
# flashcard (0.1.0-1) jammy; urgency=medium
#
#   * Initial release
#   * Raylib-based GUI
#   * CSV data format
#
#  -- game_ryo <game_ryo@ryopc.dev>  ...

# コミット
git add debian/changelog
git commit -m "debian: Release version 0.1.0"

# タグを作成（CI/CDが起動）
git tag v0.1.0
git push origin main --tags
```

### 4.2 GitHub Actions が自動実行

- `.github/workflows/publish-ppa.yml` が自動実行
- Launchpad PPA にアップロード
- GitHub Releases に `.dsc`, `.debian.tar.xz`, `.orig.tar.gz` を作成

### 4.3 ビルド状況を確認

```bash
# Launchpad のビルドページを開く
open "https://launchpad.net/~username/+archive/ubuntu/flashcard/+packages"
```

速やかで 5～30 分程度でビルド完了。

---

## Step 5: ユーザーがインストール

```bash
# PPA を追加
sudo add-apt-repository ppa:username/flashcard

# パッケージリストを更新
sudo apt-get update

# インストール
sudo apt-get install flashcard

# 実行
flashcard
```

---

## 追加バージョンのサポート

新しいバージョン 0.2.0 を公開する際：

```bash
# changelog を更新
dch -i
# Version 0.2.0 、changelog を記入

# コミット＆タグ
git add debian/changelog
git commit -m "debian: Release version 0.2.0"
git tag v0.2.0
git push origin main --tags

# 以上。CI が自動处理！
```

---

## 的一ばづをクォート（最速）

| 手順 | 必要基準 | 自動化 |
|------|---------|----------|
| 1. GPG キー作成 | ✅ 手動 | ❌ |
| 2. PPA 作成 | ✅ 手動 | ❌ |
| 3. GitHub Secrets | ✅ 手動 | ❌ |
| 4. タグをプッシュ | ✅ 粗手動で OK | ✅ GitHub Actions |
| 5. エンドユーザーインストール | ✅ 手動 | ❌ |

---

## 最不內のご誮旧

game_ryo様、上記を實行いただければ、次從 **`git tag v0.1.0` を後に推しボタンを旅程いただくだけで、以後の公開は全自動化様。

PPA を粗エリアを告附いておきたいので、きって、README.md の先頭に擁施をショーカードしたい所存です。いかがいてしょうか？
