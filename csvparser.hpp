/************************************************
 * CSVParser
 *                            tyabuta 2012.06.17
 ************************************************/

#pragma once

#include <iostream>
#include <vector>
#include <string>

/*
 * CSVファイルをセル状に分析するクラス
 * ダブルクォーテーションによる囲みや、途中の改行にも対応しています。
 * カンマ区切り対応の typedef された CSVParser がありますが、
 * basic_csvparser<'\t'> でインスタンスを作成すればTAB区切りに対応できます。
 */
template <int separate_ = ','>
class basic_csvparser{
public:
    typedef std::vector<std::string> Row;
    typedef std::vector<Row>         Cells;

private:
    enum State {
        STATE_FIRST,
        STATE_CELL_BEGIN,
        STATE_IN_CELL,
        STATE_IN_QUOTE,
        STATE_SECOND_QUOTE,
        STATE_EOF,
    };

    State       state_; // 解析中に変化していく状態フラグ
    std::string str_;   // 1Cell毎の文字列バッファ
    Cells*      data_;  // vector配列のポインタ

    unsigned string_cursor_;
    unsigned col_cursor_;
    unsigned row_cursor_;

public:
    void parse(const std::string& csv_path, Cells& ref_data){
        // 初期化
        data_  = &ref_data;
        state_ = STATE_FIRST;

        // CSVファイルのパース開始
        FILE* f;
        fopen_s(&f, csv_path.c_str(), "r");
        if (f){
            while (1){
                char c = fgetc(f);
                put(c);
                if (c == EOF) break;
            } 
            fclose(f);
        }
    }

    // ストリームに出力する。
    void output(std::ostream& out, const Cells& data){
        int row_counter = 0;
        for (auto r=data.begin(); r!=data.end(); r++, row_counter++){
            out << row_counter << ": ";

            for (auto c=r->begin(); c!=r->end(); c++){
                out << *c << " | ";
            }
            out << endl;
        }
    }

private:
    void put(char c){
        if (state_ == STATE_FIRST){
            str_.clear();
            string_cursor_ = 0;
            col_cursor_ = 0;
            row_cursor_ = 0;
            data_->clear();

            state_ = STATE_CELL_BEGIN;
        } 

        switch (state_){
        case STATE_CELL_BEGIN:
            seq_begin(c);
            break;
        case STATE_IN_CELL:
            seq_in_cell(c);
            break;
        case STATE_IN_QUOTE:
            seq_in_quote(c);
            break;
        case STATE_SECOND_QUOTE:
            seq_second_quote(c);
            break;
        }

        if (state_ == STATE_EOF){
            finish_row();
        }
    }



    void set_char(char c){
        str_ += c;
        string_cursor_++; // 文字列カーソルを進める。
    }

    void finish_cell(){
        (*data_)[row_cursor_][col_cursor_] = str_;
        str_.clear();
        string_cursor_ = 0;
        col_cursor_++;
    }

    void finish_row(){
        finish_cell();
        col_cursor_ = 0;
        row_cursor_++;
    }





    /*-------------------------------------------
      Sequence
    -------------------------------------------*/

    void seq_begin(char c){
        // Rowカーソルが移動していたら、行を追加。
        if (row_cursor_ >= data_->size()){
            data_->push_back(Row());    
        }

        // Columnカーソルが移動していたら、列を追加。
        if (col_cursor_ >= (*data_)[row_cursor_].size()){
            (*data_)[row_cursor_].push_back(std::string());
        }


        switch (c){
        case separate_: // 区切り文字なら、要素確定。
            finish_cell();
            return;

        case '\n': // 改行なら行確定。
            finish_row();
            return;

        case EOF:
            state_ = STATE_EOF;
            return;

        case '\"':
            state_ = STATE_IN_QUOTE;
            return;
        }

        set_char(c);
        state_ = STATE_IN_CELL;
    }


    void seq_in_cell(char c){
        switch (c){
        case separate_: // 区切り文字なら、要素確定。
            finish_cell();
            state_ = STATE_CELL_BEGIN;
            return;

        case '\n': // 改行なら行確定。
            finish_row();
            state_ = STATE_CELL_BEGIN;
            return;

        case EOF:
            state_ = STATE_EOF;
            return;
        }

        set_char(c);
    }

    void seq_in_quote(char c){
        switch (c){
        case EOF:
            state_ = STATE_EOF;
            return;

        case '\"':
            state_ = STATE_SECOND_QUOTE;
            return;
        }

        set_char(c);
    }

    void seq_second_quote(char c){
        switch (c){
        case separate_: // 区切り文字なら、要素確定。
            finish_cell();
            state_ = STATE_CELL_BEGIN;
            return;

        case '\n': // 改行なら行確定。
            finish_row();
            state_ = STATE_CELL_BEGIN;
            return;

        case EOF:
            state_ = STATE_EOF;
            return;
        }

        set_char(c);
        state_ = STATE_IN_QUOTE;
    }



};

typedef basic_csvparser<','> CSVParser;

