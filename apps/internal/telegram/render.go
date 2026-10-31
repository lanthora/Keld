// SPDX-License-Identifier: GPL-2.0-only
package telegram

import (
	"encoding/json"
	"fmt"
)

func RenderAuditProcReport(text string) (richText string) {
	doc := map[string]interface{}{}

	json.Unmarshal([]byte(text), &doc)
	if doc["type"].(string) != "audit::proc::report" {
		return
	}
	judge := doc["judge"].(float64)

	workdir := doc["workdir"].(string)
	binary := doc["binary"].(string)
	argv := doc["argv"].(string)

	richText += "<b>Process audit</b>\n\n"
	richText += "Working directory: "
	richText += fmt.Sprintf("<u>%s</u>\n\n", workdir)
	richText += "Executable: "
	richText += fmt.Sprintf("<u>%s</u>\n\n", binary)
	richText += "Arguments: "
	richText += fmt.Sprintf("<u>%s</u>\n\n", argv)
	richText += "Status: "
	if judge == 1 {
		richText += "<u>Success</u>\n"
	} else {
		richText += "<u>Failed</u>\n"
	}

	return
}

func RenderUserMsgSub(text string) (rich string) {
	doc := map[string]interface{}{}

	json.Unmarshal([]byte(text), &doc)
	if doc["type"].(string) != "user::msg::sub" {
		return
	}

	code := doc["code"].(float64)
	section := doc["section"].(string)

	rich += "<b>Message subscription</b>\n\n"
	rich += "Section: "
	rich += fmt.Sprintf("<u>%s</u>\n\n", section)
	rich += "Status: "
	if code == 0 {
		rich += "<u>Success</u>\n"
	} else {
		rich += "<u>Failed</u>\n"
	}
	return
}

func RenderUserMsgUnsub(text string) (rich string) {
	doc := map[string]interface{}{}

	json.Unmarshal([]byte(text), &doc)
	if doc["type"].(string) != "user::msg::unsub" {
		return
	}

	code := doc["code"].(float64)
	section := doc["section"].(string)

	rich += "<b>Message unsubscription</b>\n\n"
	rich += "Section: "
	rich += fmt.Sprintf("<u>%s</u>\n\n", section)
	rich += "Status: "
	if code == 0 {
		rich += "<u>Success</u>\n"
	} else {
		rich += "<u>Failed</u>\n"
	}
	return
}

func RenderKernelProcEnable(text string) (rich string) {
	doc := map[string]interface{}{}

	json.Unmarshal([]byte(text), &doc)
	if doc["type"].(string) != "kernel::proc::enable" {
		return
	}

	code := doc["code"].(float64)
	rich += "<b>Enable process protection</b>\n\n"

	rich += "Status: "
	if code == 0 {
		rich += "<u>Success</u>\n"
	} else {
		rich += "<u>Failed</u>\n"
	}
	return
}

func RenderKernelProcDisable(text string) (rich string) {
	doc := map[string]interface{}{}

	json.Unmarshal([]byte(text), &doc)
	if doc["type"].(string) != "kernel::proc::disable" {
		return
	}

	code := doc["code"].(float64)
	rich += "<b>Disable process protection</b>\n\n"

	rich += "Status: "
	if code == 0 {
		rich += "<u>Success</u>\n"
	} else {
		rich += "<u>Failed</u>\n"
	}
	return
}
