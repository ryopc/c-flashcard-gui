# Launchpad PPA Setup Guide

このドキュメントは、`flashcard` パッケージを Ubuntu Launchpad PPA で公開するための完全なセットアップ手順をご説明いたします。

## 前提条件

- Launchpad アカウント（https://launchpad.net で登録）
- GPG キーペア（署名用）
- `dput` コマンドラインツール
- ローカルでの `.deb` ビルド環境

## Step 1: Launchpad アカウント設定

### 1.1 GPG キーの生成（初回のみ）

```bash
# 4096-bit RSA キーを生成
gpg --full-gen-key

# 以下の情報を入力：
# - Key type: (1) RSA and RSA (default)
# - Key size: 4096
# - Validity: 0 (no expiration) または任意の期間
# - Real name: game_ryo
# - Email address: game_ryo@ryopc.dev
# - Passphrase: 安全なパスフレーズを設定
```

### 1.2 GPG キーを Launchpad にアップロード

```bash
# キー ID を確認
gpg --list-secret-keys
# 出力例: sec   rsa4096 2026-07-12 [SC]
#         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#         uid           [ultimate] game_ryo <game_ryo@ryopc.dev>

# キーを Launchpad サーバにアップロード
KEY_ID="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
gpg --send-keys --keyserver keyserver.ubuntu.com $KEY_ID
```

### 1.3 Launchpad で GPG キーを登録

1. https://launchpad.net/~/+editpgpkeys にアクセス
2. 上記でアップロードしたキー ID を入力
3. 「Import Key" をクリック
4. Launchpad からメール確認を受け取り、リンクを開く

## Step 2: PPA を Launchpad で作成

1. https://launchpad.net/~game_ryo/+ppas にアクセス（username を置き換え）
2. "Create a new PPA" をクリック
3. PPA 名を入力（例: `flashcard`）
4. 説明を追加：
   ```
   Lightweight cross-platform flashcard application built with C and Raylib
   ```
5. "Create PPA" をクリック

完成後、PPA URL は以下の形式になります：
```
https://launchpad.net/~game_ryo/+archive/ubuntu/flashcard
```

## Step 3: ローカル build 環境セットアップ

### 3.1 必要なツールをインストール

```bash
# Ubuntu/Debian
sudo apt-get install -y \
  devscripts \
  debhelper \
  dput \
  build-essential \
  libraylib-dev \
  pkg-config

# Fedora
sudo dnf install -y \
  fedora-packager \
  dput \
  raylib-devel
```

### 3.2 dput 設定

`~/.dput.cf` に以下を追加（既に存在する場合は追記）：

```ini
[ppa]
fqdn = ppa.launchpad.net
method = sftp
result_var = ppa_location
incoming = ~/ubuntu/
login = username
allow_unsigned_uploads = 0
```

`username` を Launchpad ユーザー名に置き換え。

### 3.3 GPG 設定

`~/.bashrc` または `~/.zshrc` に追加：

```bash
export DEBEMAIL="game_ryo@ryopc.dev"
export DEBFULLNAME="game_ryo"
export GPGKEY=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  # 上記のキー ID
```

## Step 4: ソースパッケージのビルド

### 4.1 リポジトリの prepare

```bash
cd c-flashcard-gui

# upstream tarball を作成（GitHub releases から自動で取得可）
git archive --format=tar.gz --prefix=flashcard-0.1.0/ v0.1.0 > ../flashcard_0.1.0.orig.tar.gz
```

### 4.2 Debian パッケージをビルド

```bash
# 変更をコミット（debian/ ディレクトリ内に있을 경우）
git add debian/
git commit -m "debian: Add PPA packaging"

# ビルド
debuild -S -sa
```

または、より簡潔に：

```bash
dbuild -S
```

### 4.3 ビルド出力確認

```bash
ls -la ../flashcard_0.1.0-1*
# 以下のファイルが生成されているか確認:
# - flashcard_0.1.0-1.dsc
# - flashcard_0.1.0-1.debian.tar.xz
# - flashcard_0.1.0.orig.tar.gz
```

## Step 5: PPA にアップロード

### 5.1 署名されたパッケージをアップロード

```bash
cd ..
dput ppa flashcard_0.1.0-1_source.changes
```

**注意:** `ppa` は dput.cf で設定した PPA エイリアスです。

### 5.2 アップロード確認

```bash
# Launchpad のビルド状況を確認
open "https://launchpad.net/~game_ryo/+archive/ubuntu/flashcard/+packages"
```

ビルド完了まで通常 **5～30 分** かかります。

## Step 6: ユーザーが PPA を追加してインストール

### 一般ユーザー向け手順

```bash
# PPA を追加
sudo add-apt-repository ppa:game_ryo/flashcard

# パッケージリストを更新
sudo apt-get update

# インストール
sudo apt-get install flashcard

# 実行
flashcard
```

## Step 7: バージョンアップデート時の手順

新しいバージョン（例: 0.2.0）をリリースする際：

```bash
# 1. changelog を更新
dch -i
# エディタが開くので、変更内容を記入

# 2. Git にコミット
git add debian/changelog
git commit -m "debian: Bump version to 0.2.0"
git tag v0.2.0

# 3. tarball を作成
git archive --format=tar.gz --prefix=flashcard-0.2.0/ v0.2.0 > ../flashcard_0.2.0.orig.tar.gz

# 4. ビルド＆アップロード
dbuild -S -sa
dput ppa ../flashcard_0.2.0-1_source.changes
```

## トラブルシューティング

### "Asc key not found" エラー

```bash
# GPG キーが Launchpad に登録されていません
# Step 1.3 を再度実行してください
```

### "Authentication failed" エラー

```bash
# SSH キー認証が必要です
ssh-keygen -t rsa -b 4096 -C "game_ryo@ryopc.dev"
# 生成されたキーを Launchpad (~/.ssh/id_rsa.pub の内容を
# https://launchpad.net/~/+editsshkeys にコピー)
```

### ビルド失敗（"Build failed"）

```bash
# Launchpad のビルドログを確認
# https://launchpad.net/~game_ryo/+archive/ubuntu/flashcard/+packages
# 該当パッケージのログをクリック
```

## 参考資料

- [Ubuntu PPA Official Documentation](https://help.launchpad.net/Packaging/PPA)
- [Debian New Maintainers' Guide](https://www.debian.org/doc/manuals/maint-guide/)
- [Launchpad Developers](https://help.launchpad.net/)
