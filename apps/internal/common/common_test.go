// SPDX-License-Identifier: GPL-2.0-only
package common

import (
	"context"
	"database/sql"
	"path/filepath"
	"testing"

	"github.com/spf13/viper"
	_ "modernc.org/sqlite"
)

func TestDatabaseConnectionOptions(t *testing.T) {
	config := viper.New()
	config.Set("db", filepath.Join(t.TempDir(), "keld.db"))
	dsn := GetDataSourceNameFromConfig(config)
	db, err := sql.Open("sqlite", dsn)
	if err != nil {
		t.Fatal(err)
	}
	defer db.Close()
	// Check WAL mode with multiple live pool connections.
	for i := 0; i < 2; i++ {
		conn, err := db.Conn(context.Background())
		if err != nil {
			t.Fatal(err)
		}
		defer conn.Close()
		var mode string
		if err := conn.QueryRowContext(context.Background(), "PRAGMA journal_mode").Scan(&mode); err != nil {
			t.Fatal(err)
		}
		if mode != "wal" {
			t.Fatalf("journal_mode = %q, want wal", mode)
		}
	}
	if _, err := db.Exec("CREATE TABLE compatibility (id INTEGER PRIMARY KEY, value TEXT)"); err != nil {
		t.Fatal(err)
	}
	if _, err := db.Exec("INSERT INTO compatibility VALUES (1, 'persisted value')"); err != nil {
		t.Fatal(err)
	}
	reopened, err := sql.Open("sqlite", dsn)
	if err != nil {
		t.Fatal(err)
	}
	defer reopened.Close()
	var value string
	if err := reopened.QueryRow("SELECT value FROM compatibility WHERE id = 1").Scan(&value); err != nil {
		t.Fatal(err)
	}
	if value != "persisted value" {
		t.Fatalf("value = %q", value)
	}
	var integrity string
	if err := reopened.QueryRow("PRAGMA integrity_check").Scan(&integrity); err != nil {
		t.Fatal(err)
	}
	if integrity != "ok" {
		t.Fatalf("integrity_check = %q", integrity)
	}
}
